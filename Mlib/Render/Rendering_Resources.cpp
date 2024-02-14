#include "Rendering_Resources.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Texture/Pack_Boxes.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
#include <Mlib/Images/Image_Info.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Is_Power_Of_Two.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>
#include <Mlib/Render/Gl_Extensions.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Texture_Atlas.hpp>
#include <Mlib/Render/Render_To_Texture/Render_To_Texture_2D.hpp>
#include <Mlib/Render/Render_To_Texture/Render_To_Texture_2D_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Text/Loaded_Font.hpp>
#include <Mlib/Render/Viewport_Guard.hpp>
#include <Mlib/Threads/Recursion_Guard.hpp>
#include <Mlib/Threads/Thread_Local.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <nv_dds/nv_dds.hpp>
#include <stb/stb_image_resize.h>
#include <stb/stb_image_write.h>
#include <stb_cpp/stb_alpha_fac.hpp>
#include <stb_cpp/stb_array.hpp>
#include <stb_cpp/stb_blend.hpp>
#include <stb_cpp/stb_colorize.hpp>
#include <stb_cpp/stb_desaturate.hpp>
#include <stb_cpp/stb_generate_color_mask.hpp>
#include <stb_cpp/stb_image_atlas.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <stb_cpp/stb_invert.hpp>
#include <stb_cpp/stb_lighten.hpp>
#include <stb_cpp/stb_mipmaps.hpp>
#include <stb_cpp/stb_set_alpha.hpp>
#include <stb_cpp/stb_transform.hpp>
#include <stb_cpp/stb_truetype_aligned.hpp>
#include <string>
#include <vector>

using namespace Mlib;
namespace fs = std::filesystem;

#ifdef __ANDROID__
static const bool EXTRACT_PROCESSED = false;
static const bool EXTRACT_RAW = false;
#else
static const bool EXTRACT_PROCESSED = true;
static const bool EXTRACT_RAW = false;
#endif

template <class TNode>
class ValueExtractor {
public:
    explicit ValueExtractor(TNode&& node)
        : node_{ std::move(node) }
    {}
    auto& operator * () {
        return node_.mapped();
    }
    auto* operator -> () {
        return &node_.mapped();
    }
    bool operator == (std::nullptr_t) const {
        return node_.empty();
    }
private:
    TNode node_;
};

template <bool textract, class TContainer, class TKey>
auto get_or_extract(TContainer& container, const TKey& key) {
    if constexpr (textract) {
        return ValueExtractor{ container.try_extract(key) };
    } else {
        return container.try_get(key);
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const AutoAtlasTileDescriptor& aatd) {
    ostr <<
        "name: " << aatd.filename <<
        " position: (" << aatd.left << ", " << aatd.bottom <<
        ") size: (" << aatd.width << ", " << aatd.height << ')';
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const AutoTextureAtlasDescriptor& atad) {
    ostr <<
        "width: " << atad.width <<
        " height: " << atad.height <<
        " mip_level_count: " << atad.mip_level_count <<
        " color_mode: " << color_mode_to_string(atad.color_mode);
    for (const auto& [layer, tiles] : enumerate(atad.tiles)) {
        ostr << "  layer: " << layer << '\n';
        for (const auto& tile : tiles) {
            ostr << "  " << tile << '\n';
        }
    }
    return ostr;
}

/**
 * From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
 */
int log2(int n) {
    int result = 0;
    while (n >>= 1) ++result;
    return result;
}

static StbInfo<uint8_t> stb_load_texture(
    const std::string& filename,
    int nchannels,
    FlipMode flip_mode)
{
    auto result = stb_load8(filename, flip_mode, nullptr, IncorrectDatasizeBehavior::CONVERT);
    if (result.nrChannels < std::abs(nchannels)) {
        THROW_OR_ABORT(filename + " does not have at least " + std::to_string(nchannels) + " channels");
    }
    if (!is_power_of_two(result.width) || !is_power_of_two(result.height)) {
        lwarn() << filename << " size: " << result.width << 'x' << result.height;
    }
    if ((nchannels > 0) && (result.nrChannels != nchannels)) {
        lwarn() << filename << " channels: " << result.nrChannels << ", expected: " << nchannels;
    }
    return result;
}

static StbInfo<uint8_t> stb_load_and_transform_texture(const ColormapWithModifiers& color, FlipMode flip_mode) {
    std::string touch_file = color.filename + ".xpltd";
    bool has_color_selector =
        (color.selected_color_near != 0.f) ||
        (color.selected_color_far != INFINITY);
    auto source_color_mode = has_color_selector
        ? ColorMode::RGB
        : color.color_mode;
    if (has_color_selector && (color.color_mode != ColorMode::GRAYSCALE)) {
        THROW_OR_ABORT("Color-selector requires grayscale");
    }
    if ((source_color_mode == ColorMode::RGBA) &&
        color.alpha.empty() &&
        getenv_default_bool("EXTRAPOLATE_COLORS", false) &&
        !fs::exists(touch_file))
    {
        linfo() << "Extrapolating RGBA image \"" << color << '"';
        auto img = StbImage4::load_from_file(color.filename);
        float sigma = 3.f;
        size_t niterations = 1 + (size_t)((float)std::max(img.shape(0), img.shape(1)) / (sigma * 4));
        extrapolate_rgba_colors(
            img,
            sigma,
            niterations).save_to_file(color.filename);
        std::ofstream ofstr{ touch_file };
        if (ofstr.fail()) {
            THROW_OR_ABORT("Could not create file \"" + touch_file + '"');
        }
    }
    StbInfo<uint8_t> si0;
    if (!color.alpha.empty()) {
        if (source_color_mode != ColorMode::RGBA) {
            THROW_OR_ABORT("Color mode not RGBA despite alpha texture: \"" + color.filename + '"');
        }
        si0 = stb_load_texture(
            color.filename, (int)ColorMode::RGB, flip_mode);
        if (si0.nrChannels != 3) {
            THROW_OR_ABORT("#channels not 3: \"" + color.filename + '"');
        }
        auto si_alpha = stb_load_texture(
            color.alpha, (int)ColorMode::GRAYSCALE, flip_mode);
        if (si_alpha.nrChannels != 1) {
            THROW_OR_ABORT("#channels not 1: \"" + color.alpha + '"');
        }
        StbInfo<uint8_t> si0_rgb = std::move(si0);
        si0 = stb_create<uint8_t>(si0_rgb.width, si0_rgb.height, 4);
        stb_set_alpha(
            si0_rgb.data.get(),
            si_alpha.data.get(),
            si0.data.get(),
            si0.width,
            si0.height,
            si_alpha.width,
            si_alpha.height);
    } else {
        si0 = stb_load_texture(
            color.filename, (int)source_color_mode, flip_mode);
    }
    if (!color.average.empty()) {
        auto si1 = stb_load_texture(
            color.average, (int)source_color_mode, flip_mode);
        stb_average(
            si0.data.get(),
            si1.data.get(),
            si0.data.get(),
            si0.width,
            si0.height,
            si1.width,
            si1.height,
            si0.nrChannels,
            si1.nrChannels,
            si0.nrChannels);
    }
    if (!color.multiply.empty()) {
        auto si1 = stb_load_texture(
            color.multiply, (int)source_color_mode, flip_mode);
        stb_multiply_color(
            si0.data.get(),
            si1.data.get(),
            si0.data.get(),
            si0.width,
            si0.height,
            si1.width,
            si1.height,
            si0.nrChannels,
            si1.nrChannels,
            si0.nrChannels);
    }
    if (color.alpha_fac != 1.f) {
        stb_alpha_fac(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            color.alpha_fac);
    }
    if (color.desaturate != 0.f) {
        stb_desaturate(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            color.desaturate);
    }
    if (!color.histogram.empty()) {
        Array<unsigned char> image = stb_image_2_array(si0);
        Array<unsigned char> ref = stb_image_2_array(stb_load_texture(color.histogram, -3, FlipMode::NONE));
        Array<unsigned char> m = match_rgba_histograms(image, ref);
        assert_true(m.shape(0) == (size_t)si0.nrChannels);
        assert_true(m.shape(1) == (size_t)si0.height);
        assert_true(m.shape(2) == (size_t)si0.width);
        array_2_stb_image(m, si0.data.get());
    }
    if (!color.lighten.all_equal(0.f)) {
        const FixedArray<float, 3>& lighten = color.lighten;
        if (any(lighten > 1.f) ||
            any(lighten < -1.f) ||
            !all(Mlib::isfinite(lighten)))
        {
            THROW_OR_ABORT("Lighten value out of bounds");
        }
        stb_lighten(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.lighten * 255.f).casted<short>().flat_begin());
    }
    if (!color.lighten_left.all_equal(0.f) ||
        !color.lighten_right.all_equal(0.f))
    {
        const FixedArray<float, 3>& lighten_left = color.lighten_left;
        const FixedArray<float, 3>& lighten_right = color.lighten_right;
        if (!all(Mlib::isfinite(lighten_left)) ||
            any(lighten_left > 1.f) ||
            any(lighten_left < -1.f))
        {
            THROW_OR_ABORT("Lighten top value out of bounds");
        }
        if (!all(Mlib::isfinite(lighten_right)) ||
            any(lighten_right > 1.f) ||
            any(lighten_right < -1.f))
        {
            THROW_OR_ABORT("Lighten bottom value out of bounds");
        }
        stb_lighten_horizontal_gradient(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.lighten_left * 255.f).casted<short>().flat_begin(),
            (color.lighten_right * 255.f).casted<short>().flat_begin());
    }
    if (!color.lighten_top.all_equal(0.f) ||
        !color.lighten_bottom.all_equal(0.f))
    {
        const FixedArray<float, 3>& lighten_top = color.lighten_top;
        const FixedArray<float, 3>& lighten_bottom = color.lighten_bottom;
        if (!all(Mlib::isfinite(lighten_top)) ||
            any(lighten_top > 1.f) ||
            any(lighten_top < -1.f))
        {
            THROW_OR_ABORT("Lighten top value out of bounds");
        }
        if (!all(Mlib::isfinite(lighten_bottom)) ||
            any(lighten_bottom > 1.f) ||
            any(lighten_bottom < -1.f))
        {
            THROW_OR_ABORT("Lighten bottom value out of bounds");
        }
        stb_lighten_vertical_gradient(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.lighten_top * 255.f).casted<short>().flat_begin(),
            (color.lighten_bottom * 255.f).casted<short>().flat_begin());
    }
    if (!color.mean_color.all_equal(-1.f)) {
        if (!stb_colorize(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            lwarn() << "alpha = 0: " << color << std::endl;
        }
    }
    if (!color.alpha_blend.empty()) {
        auto si1 = stb_load_texture(
            color.alpha_blend, 4, flip_mode);
        stb_alpha_blend(
            si0.data.get(),
            si1.data.get(),
            si0.data.get(),
            si0.width,
            si0.height,
            si1.width,
            si1.height,
            si0.nrChannels,
            si1.nrChannels,
            si0.nrChannels);
    }
    if (has_color_selector)
    {
        if (si0.nrChannels != 3) {
            THROW_OR_ABORT("Only 3 channels are supported for selected_color");;
        }
        if (color.color_mode != ColorMode::GRAYSCALE) {
            THROW_OR_ABORT("Color-selector requires grayscale");
        }
        const FixedArray<float, 3>& selected_color = color.selected_color;
        if (any(selected_color < 0.f) || any(selected_color > 1.f)) {
            THROW_OR_ABORT("selected_color out of bounds");
        }
        if (!std::isfinite(color.selected_color_near) ||
            (color.selected_color_near < 0.f) ||
            (color.selected_color_near > 1.f))
        {
            THROW_OR_ABORT("selected_color_near out of bounds");
        }
        if (!std::isfinite(color.selected_color_far) ||
            (color.selected_color_far < 0.f) ||
            (color.selected_color_far > 1.f))
        {
            THROW_OR_ABORT("selected_color_far out of bounds");
        }
        auto si1 = stb_create<uint8_t>(si0.width, si0.height, 1);
        assert_isequal(si0.nrChannels, integral_cast<int>(color.selected_color.length()));
        stb_generate_color_mask(
            si0.data.get(),
            si1.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.selected_color * 255.f).casted<short>().flat_begin(),
            (unsigned short)std::round(color.selected_color_near * 255.f),
            (unsigned short)std::round(color.selected_color_far * 255.f));
        si0 = std::move(si1);
    }
    if ((color.times != 1.f) || (color.plus != 0.f)) {
        stb_transform(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            color.times,
            color.plus,
            color.abs);
    }
    if (color.invert) {
        stb_invert(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels);
    }
    return si0;
}

static double mean_opacity(const StbInfo<uint8_t>& si) {
    if (si.nrChannels != 4) {
        THROW_OR_ABORT("warn_if_invisible received image that does not have 4 channels");
    }
    double opacity = 0.f;
    for (int r = 0; r < si.height; ++r) {
        for (int c = 0; c < si.width; ++c) {
            opacity += double(si.data.get()[(r * si.width + c) * si.nrChannels + 3]) / 255.;
        }
    }
    return opacity / double(si.width * si.height);
}

static void check_color_mode(
    const ColormapWithModifiers& color,
    TextureRole role)
{
    if ((role == TextureRole::COLOR) || (role == TextureRole::COLOR_FROM_DB)) {
        if (color.color_mode == ColorMode::UNDEFINED) {
            THROW_OR_ABORT("Colormode undefined in color texture \"" + color.filename + '"');
        }
    } else if (role == TextureRole::SPECULAR) {
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("Colormode not RGB in specularmap: \"" + color.filename + '"');
        }
    } else if (role == TextureRole::NORMAL) {
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("Colormode not RGB in normalmap: \"" + color.filename + '"');
        }
    } else if (role == TextureRole::TRUSTED) {
        // Do nothing
    } else {
        THROW_OR_ABORT("Unknown texture role: " + std::to_string((int)role));
    }
}

// static void generate_rgba_mipmaps_inplace(const StbInfo& si) {
//     if (!is_power_of_two(si.width) || !is_power_of_two(si.height)) {
//         THROW_OR_ABORT("Image size is not a power of 2");
//     }
//     assert_true(si.nrChannels == 4);
//     CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, log2(std::max(si.width, si.height))));
// 
//     int level = 0;
//     RgbaDownsampler rds{si.data.get(), si.width, si.height};
//     for (RgbaImage im = rds.next(); im.data != nullptr; im = rds.next()) {
//         glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/a/49126350/2292832
//         CHK(glTexImage2D(GL_TEXTURE_2D, level++, GL_RGBA, im.width, im.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, im.data));
//     }
//     assert_true(level - 1 == log2(std::max(si.width, si.height)));
// }

void RenderingResources::print(std::ostream& ostr, size_t indentation) const {
    std::shared_lock lock{mutex_};
    std::string indent = std::string(indentation, ' ');
    ostr << indent << "Name: " << name_ << '\n';
    ostr << indent << "Texture descriptors\n";
    for (const auto& x : texture_descriptors_) {
        ostr << indent << "  " << x.first << '\n';
    }
    ostr << indent << "Blend map textures\n";
    for (const auto& x : blend_map_textures_) {
        ostr << indent << "  " << x.first << '\n';
    }
    ostr << indent << "Textures\n";
    for (const auto& x : textures_) {
        ostr << indent << "  " << x.first << '\n';
    }
    ostr << indent << "vps\n";
    for (const auto& x : vps_) {
        ostr << indent << "  " << x.first << '\n';
    }
    ostr << indent << "Discreteness\n";
    for (const auto& x : discreteness_) {
        ostr << indent << "  " << x.first << '\n';
    }
    ostr << indent << "Texture wrap\n";
    for (const auto& x : texture_wrap_) {
        ostr << indent << "  " << x.first << '\n';
    }
}

RenderingResources::RenderingResources(std::string name,
                                       unsigned int max_anisotropic_filtering_level)
    : preloaded_processed_texture_data_{"Preloaded processed texture data",
                                        [](const ColormapWithModifiers &e) { return e.filename; }}
    , preloaded_raw_texture_data_{"Preloaded raw texture data"}
    , preloaded_texture_dds_data_{"Preloaded texture DDS data"}
    , texture_descriptors_{"Texture descriptor"}
    , textures_{"Texture", [](const ColormapWithModifiers &e) { return e.filename; }}
    , manual_atlas_tile_descriptors_{"Manual atlas tile descriptor"}
    , auto_atlas_tile_descriptors_{"Auto atlas tile descriptor"}
    , cubemap_descriptors_{"Cubemap descriptor"}
    , font_textures_{"Font", [](const auto &e) { return e.first; }}
    , vps_{"VP"}
    , offsets_{"Offset"}
    , discreteness_{"Discreteness"}
    , scales_{"Scale"}
    , texture_wrap_{"Texture wrap"}
    , blend_map_textures_{"Blend-map texture"}
    , render_programs_{"Render program", [](const RenderProgramIdentifier &e) { return "<RPI>"; }}
    , name_{std::move(name)}
    , max_anisotropic_filtering_level_{max_anisotropic_filtering_level}
    , preloader_background_loop_{"Preload_BG"}
    , deallocation_token_{render_deallocator.insert([this]() {
        for (const auto& [d, _] : textures_) {
            append_render_allocator([this, d=d](){ preload(d, TextureRole::TRUSTED); });
        }
        deallocate();
    })} {
}

RenderingResources::~RenderingResources() {
    deallocate();
}

void RenderingResources::deallocate() {
    std::scoped_lock lock{mutex_};
    render_programs_.clear();
    textures_.erase_if([](auto &item) {
        auto &[_, texture] = item;
        if (texture.owner == ResourceOwner::CONTAINER) {
            try_delete_texture(texture.handle);
            return true;
        }
        return false;
    });
    font_textures_.clear();
}

void RenderingResources::preload(const TextureDescriptor& descriptor) const {
    auto dit = texture_descriptors_.try_get(descriptor.color.filename);
    const TextureDescriptor& desc = dit != nullptr
        ? *dit
        : descriptor;
    if (!desc.color.filename.empty()) {
        preload(desc.color, TextureRole::COLOR);
    }
    if (!desc.specular.filename.empty()) {
        preload(desc.specular, TextureRole::SPECULAR);
    }
    if (!desc.normal.filename.empty()) {
        preload(desc.normal, TextureRole::NORMAL);
    }
}

void RenderingResources::preload(const ColormapWithModifiers& color, TextureRole role) const {
    LOG_FUNCTION("RenderingResources::preload, color=" + color);
    if (color.filename.empty()) {
        THROW_OR_ABORT("Attempt to preload empty texture");
    }
    if (textures_.contains(color)) {
        return;
    }
    if (ContextQuery::is_initialized()) {
        get_texture(color, role, CallerType::PRELOAD);
    } else {
        check_color_mode(color, role);
        {
            std::scoped_lock lock{ mutex_ };
            if (textures_.contains(color)) {
                return;
            }
            if (!preloaded_processed_texture_data_.contains(color) &&
                !preloaded_raw_texture_data_.contains(color.filename) &&
                !preloaded_texture_dds_data_.contains(color.filename) &&
                !auto_atlas_tile_descriptors_.contains(color.filename))
            {
                auto data = get_texture_data(color, role, FlipMode::VERTICAL);
                preloaded_processed_texture_data_.emplace(color, std::move(data));
                if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                    linfo() << this << " Preloaded texture: " << color;
                }
            }
        }
        append_render_allocator([this, color, role]() { get_texture(color, role, CallerType::PRELOAD); });
    }
}

bool RenderingResources::texture_is_loaded_and_try_preload(
    const ColormapWithModifiers& color,
    TextureRole role)
{
    if (texture_is_loaded_unsafe(color)) {
        return true;
    }
    if (!preloader_background_loop_.done()) {
        return false;
    }
    std::scoped_lock lock{mutex_};
    if (texture_is_loaded_unsafe(color)) {
        return true;
    }
    if (preloader_background_loop_.done()) {
        preloader_background_loop_.run([this, color, role](){
            preload(color, role);
        });
    }
    return false;
}

bool RenderingResources::texture_is_loaded_unsafe(const ColormapWithModifiers& name) const {
    if (preloaded_texture_dds_data_.contains(name.filename)) {
        return true;
    }
    if (preloaded_processed_texture_data_.contains(name)) {
        return true;
    }
    if (preloaded_raw_texture_data_.contains(name.filename)) {
        return true;
    }
    if (textures_.contains(name)) {
        return true;
    }
    if (auto_atlas_tile_descriptors_.contains(name.filename)) {
        THROW_OR_ABORT("Attempted lazy access to auto texture atlas");
    }
    return false;
}

bool RenderingResources::contains_texture(const ColormapWithModifiers& name) const {
    std::shared_lock lock{mutex_};
    return textures_.contains(name) || auto_atlas_tile_descriptors_.contains(name.filename);
}

const ColormapWithModifiers& RenderingResources::colormap(const ColormapWithModifiers& name) const
{
    if (auto dit = texture_descriptors_.try_get(name.filename); dit != nullptr) {
        return dit->color;
    } else {
        return name;
    }
}

std::string RenderingResources::get_texture_filename(
    const ColormapWithModifiers& color,
    TextureRole role,
    const std::string& default_filename) const
{
    std::shared_lock lock{mutex_};
    
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture_filename(colormap(color), TextureRole::COLOR, default_filename);
    }

    check_color_mode(color, role);

    if (manual_atlas_tile_descriptors_.contains(color.filename) ||
        (color.desaturate != 0.f) ||
        !color.histogram.empty() ||
        !color.average.empty() ||
        !color.multiply.empty() ||
        !color.mean_color.all_equal(-1.f) ||
        !color.lighten.all_equal(0.f))
    {
        StbInfo si = get_texture_data(color, role, FlipMode::VERTICAL);
        if (!default_filename.ends_with(".png")) {
            THROW_OR_ABORT("Filename \"" + default_filename + "\" does not end with .png");
        }
        if (!stbi_write_png(
            default_filename.c_str(),
            si.width,
            si.height,
            si.nrChannels,
            si.data.get(),
            0))
        {
            THROW_OR_ABORT("Could not save to file \"" + default_filename + '"');
        }
        return default_filename;
    } else {
        return color.filename;
    }
}

static GLenum nchannels2sized_internal_format(size_t nchannels) {
    switch (nchannels) {
        case 1:
            return GL_R8;
        case 3:
            return GL_RGB8;
        case 4:
            return GL_RGBA8;
        default:
            THROW_OR_ABORT("Unsupported number of channels: " + std::to_string(nchannels));
    };
}

static GLint nchannels2internal_format(size_t nchannels) {
    switch (nchannels) {
        case 1:
            return GL_R8;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            THROW_OR_ABORT("Unsupported number of channels: " + std::to_string(nchannels));
    };
}

static GLenum nchannels2format(size_t nchannels) {
    switch (nchannels) {
        case 1:
            return GL_RED;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            THROW_OR_ABORT("Unsupported number of channels: " + std::to_string(nchannels));
    };
}

GLuint RenderingResources::get_texture(
    const ColormapWithModifiers& color,
    TextureRole role,
    CallerType caller_type) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture(colormap(color), TextureRole::COLOR, caller_type);
    }
    check_color_mode(color, role);
    LOG_FUNCTION("RenderingResources::get_texture " + color.filename);
    if (auto it = textures_.try_get(color); it != nullptr) {
        return it->handle;
    }
    std::scoped_lock lock{mutex_};
    if (auto it = textures_.try_get(color); it != nullptr) {
        return it->handle;
    }
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    RecursionGuard rg{recursion_counter};
    if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
        linfo() << this << " Loading texture: " << color;
        if (!color.average.empty()) {
            linfo() << this << " Loading texture: " << color.average;
        }
        if (!color.multiply.empty()) {
            linfo() << this << " Loading texture: " << color.multiply;
        }
    }

    float aniso = 0.0f;
    WARN(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
    aniso = std::min({ aniso, (float)color.anisotropic_filtering_level, (float)max_anisotropic_filtering_level_ });

    GLuint texture;

    if (auto aptr = get_or_extract<EXTRACT_PROCESSED>(auto_atlas_tile_descriptors_, color.filename); aptr != nullptr) {
        if (caller_type != CallerType::PRELOAD) {
            THROW_OR_ABORT("Texture source is not preload for texture \"" + color.filename + '"');
        }
        texture = render_to_texture_2d_array(
            aptr->width,
            aptr->height,
            integral_cast<GLsizei>(aptr->tiles.size()),
            aptr->mip_level_count,
            aniso,
            nchannels2sized_internal_format(integral_cast<size_t>((int)aptr->color_mode)),
            [this, &aptr](GLsizei width, GLsizei height, GLsizei layer)
            {
                render_texture_atlas(
                    *const_cast<RenderingResources*>(this),
                    aptr->tiles.at(integral_cast<size_t>(layer)),
                    integral_to_float<float>(width) / integral_to_float<float>(aptr->width),
                    integral_to_float<float>(height) / integral_to_float<float>(aptr->height));
            });
    } else {
        if (preloaded_texture_dds_data_.contains(color.filename)) {
            GLuint original_texture;
            CHK(glGenTextures(1, &original_texture));
            DestructionGuard dg0{ [original_texture]() { ABORT(glDeleteTextures(1, &original_texture)); } };
            auto sinfo = [&](){
                CHK(glBindTexture(GL_TEXTURE_2D, original_texture));
                DestructionGuard dg1{ []() { ABORT(glBindTexture(GL_TEXTURE_2D, 0)); } };
                return initialize_dds_texture(color);
            }();
            auto original_key = ColormapWithModifiers{
                .filename = "__original_texture__",
                .color_mode = (ColorMode)sinfo.nchannels,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS
            };
            const_cast<RenderingResources*>(this)->set_texture(
                original_key,
                original_texture,
                ResourceOwner::CALLER);
            DestructionGuard dg1{ [this, &original_key]() { const_cast<RenderingResources*>(this)->delete_texture(original_key, DeletionFailureMode::ABORT); } };

            FillWithTextureLogic logic{
                *const_cast<RenderingResources*>(this),
                original_key,
                ResourceUpdateCycle::ONCE,
                CullFaceMode::CULL,
                AlphaChannelRole::NO_BLEND,
                vertically_flipped_quad_vertices };
            texture = render_to_texture_2d(
                sinfo.width,
                sinfo.height,
                // https://stackoverflow.com/questions/9572414/how-many-mipmaps-does-a-texture-have-in-opengl
                (sinfo.mip_level_count == 0)
                    ? float_to_integral<GLsizei>(std::floor(std::log2(std::max(sinfo.width, sinfo.height))))
                    : sinfo.mip_level_count,
                aniso,
                nchannels2sized_internal_format(integral_cast<size_t>(sinfo.nchannels)),
                [&logic](GLsizei width, GLsizei height)
                {
                    ViewportGuard vg{
                        0.f,
                        0.f,
                        integral_to_float<float>(width),
                        integral_to_float<float>(height)};
                    logic.render();
                });
        } else if (cubemap_descriptors_.contains(color.filename)) {
            texture = get_cubemap_unsafe(color.filename);
        } else {
            CHK(glGenTextures(1, &texture));
            CHK(glBindTexture(GL_TEXTURE_2D, texture));
            if (aniso != 0) {
                CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
            }
            initialize_non_dds_texture(color, role);
            CHK(glBindTexture(GL_TEXTURE_2D, 0));
        }
    }

    textures_.emplace(color, TextureHandleAndOwner{texture, ResourceOwner::CONTAINER});
    return texture;
}

GLuint RenderingResources::get_cubemap_unsafe(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_cubemap " + name);
    auto it = cubemap_descriptors_.get(name);
    if (it.filenames.size() != 6) {
        THROW_OR_ABORT("Cubemap does not have 6 filenames");
    }
    GLuint textureID;
    CHK(glGenTextures(1, &textureID));
    CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    for (GLuint i = 0; i < it.filenames.size(); i++) {
        auto color = it.filenames[i];
        auto dit = texture_descriptors_.try_get(color);
        ColormapWithModifiers desc = dit != nullptr
            ? dit->color
            : ColormapWithModifiers{
                .filename = color,
                .color_mode = ColorMode::RGB};

        auto info = get_texture_data(desc, TextureRole::COLOR, FlipMode::NONE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/a/49126350/2292832
        CHK(glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                            0,
                            GL_RGB,
                            info.width,
                            info.height,
                            0,
                            GL_RGB,
                            GL_UNSIGNED_BYTE,
                            info.data.get()));
    }
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    return textureID;
}

void RenderingResources::set_texture(const ColormapWithModifiers& name, GLuint id, ResourceOwner resource_owner) {
    LOG_FUNCTION("RenderingResources::set_texture " + name.filename);
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        THROW_OR_ABORT("RenderingResources::set_texture: invalid texture ID");
    }
    std::scoped_lock lock{ mutex_ };
    if (auto v = textures_.try_get(name); v != nullptr) {
        if (v->owner == ResourceOwner::CONTAINER) {
            THROW_OR_ABORT("Overwriting texture that needs garbage-collection");
        }
        textures_.erase(name);
    }
    textures_.emplace(
        name,
        TextureHandleAndOwner{
            .handle = id,
            .owner = resource_owner});
}

void RenderingResources::add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + name);
    if (descriptor.color.color_mode == ColorMode::UNDEFINED) {
        THROW_OR_ABORT("Colormode undefined color texture: \"" + descriptor.color.filename + '"');
    }
    if (!descriptor.specular.filename.empty() && (descriptor.specular.color_mode != ColorMode::RGB)) {
        THROW_OR_ABORT("Colormode not RGB in specularmap: \"" + descriptor.specular.filename + '"');
    }
    if (!descriptor.normal.filename.empty() && (descriptor.normal.color_mode != ColorMode::RGB)) {
        THROW_OR_ABORT("Colormode not RGB in normalmap: \"" + descriptor.normal.filename + '"');
    }
    texture_descriptors_.emplace(name, descriptor);
}

TextureDescriptor RenderingResources::get_existing_texture_descriptor(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_existing_texture_descriptor " + name);
    return texture_descriptors_.get(name);
}

void RenderingResources::add_manual_texture_atlas(
    const std::string& name,
    const ManualTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_manual_texture_atlas " + name);
    std::scoped_lock lock{mutex_};
    manual_atlas_tile_descriptors_.emplace(name, texture_atlas_descriptor);
}

void RenderingResources::add_auto_texture_atlas(
    const std::string& name,
    const AutoTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_auto_texture_atlas " + name);
    std::scoped_lock lock{mutex_};
    append_render_allocator(
        [this, name, texture_atlas_descriptor]()
        {
            preload(
                {
                    .filename = name,
                    .color_mode = texture_atlas_descriptor.color_mode,
                    .mipmap_mode = MipmapMode::WITH_MIPMAPS
                },
                TextureRole::COLOR);
        });
    auto_atlas_tile_descriptors_.emplace(name, texture_atlas_descriptor);
}

void RenderingResources::add_cubemap(const std::string& name, const std::vector<std::string>& filenames) {
    LOG_FUNCTION("RenderingResources::add_cubemap " + name);
    std::scoped_lock lock{mutex_};
    if (textures_.contains({.filename = name})) {
        THROW_OR_ABORT("Texture with name \"" + name + "\" already exists");
    }
    cubemap_descriptors_.emplace(name, CubemapDescriptor{.filenames = filenames});
}

StbInfo<uint8_t> RenderingResources::get_texture_data(
    const ColormapWithModifiers& color,
    TextureRole role,
    FlipMode flip_mode,
    CopyBehavior copy_behavior) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture_data(colormap(color), TextureRole::COLOR, flip_mode, copy_behavior);
    }
    check_color_mode(color, role);
    if (auto it = manual_atlas_tile_descriptors_.try_get(color.filename); it != nullptr) {
        auto si = stb_create<uint8_t>(it->width, it->height, (int)it->color_mode);
        std::vector<AtlasTile> atlas_tiles;
        atlas_tiles.reserve(it->tiles.size());
        for (const auto& atd : it->tiles) {
            auto dit = texture_descriptors_.try_get(atd.filename);
            ColormapWithModifiers desc = dit != nullptr
                ? dit->color
                : ColormapWithModifiers{
                    .filename = atd.filename,
                    .color_mode = color.color_mode};
            atlas_tiles.push_back(AtlasTile{
                .left = atd.left,
                .bottom = atd.bottom,
                .image = get_texture_data(desc, role, flip_mode)});
        }
        build_image_atlas(si, atlas_tiles);
        return si;
    }
    if (auto it = preloaded_texture_dds_data_.try_get(color.filename); it != nullptr) {
        auto info = ImageInfo::load(color.filename, it);
        FrameBuffer fb;
        fb.configure(FrameBufferConfig{
            .width = integral_cast<int>(info.size(0)),
            .height = integral_cast<int>(info.size(1)),
            .color_internal_format = GL_RGBA,
            .color_format = GL_RGBA,
            .color_filter_type = GL_NEAREST,
            .depth_kind = FrameBufferChannelKind::NONE});
        ViewportGuard vg{
            (float)0,
            (float)0,
            (float)info.size(0),
            (float)info.size(1)};
        FillWithTextureLogic logic{
            *const_cast<RenderingResources*>(this),
            color,
            ResourceUpdateCycle::ONCE,
            CullFaceMode::CULL,
            AlphaChannelRole::NO_BLEND};
        {
            RenderToFrameBufferGuard rfg{fb};
            logic.render();
        }
        return fb.color_to_stb_image();
    }
    if (auto it = preloaded_processed_texture_data_.try_get(color); it != nullptr) {
        if (copy_behavior == CopyBehavior::RAISE) {
            THROW_OR_ABORT("Refusing to copy \"" + color.filename + '"');
        }
        auto result = stb_create<uint8_t>(it->width, it->height, it->nrChannels);
        std::copy(it->data.get(), it->data.get() + it->width * it->height * it->nrChannels, result.data.get());
        return result;
    }
    if (auto it = preloaded_raw_texture_data_.try_get(color.filename); it != nullptr) {
        if (copy_behavior == CopyBehavior::RAISE) {
            THROW_OR_ABORT("Refusing to copy \"" + color.filename + '"');
        }
        return stb_load8(color.filename, FlipMode::NONE, it, IncorrectDatasizeBehavior::CONVERT);
    }
    auto si = stb_load_and_transform_texture(color, flip_mode);
    if ((color.color_mode == ColorMode::RGB) &&
        (si.nrChannels == 4) &&
        getenv_default_bool("CHECK_OPACITY", false))
    {
        double opacity = mean_opacity(si);
        if (opacity < 0.8) {
            lwarn() << color << ": Opacity is only " << opacity;
        }
    }
    return si;
}

std::map<std::string, ManualUvTile> RenderingResources::generate_manual_texture_atlas(
    const std::string& name,
    const std::vector<std::string>& filenames)
{
    std::map<std::string, FixedArray<int, 2>> texture_sizes;
    for (const auto& filename : filenames) {
        int x;
        int y;
        int comp;
        if (stbi_info(filename.c_str(), &x, &y, &comp) == 0) {
            THROW_OR_ABORT("Could not read size information from file \"" + filename + '"');
        }
        texture_sizes[filename] = {x, y};
    }
    ManualTextureAtlasDescriptor tad{
        .width = 0,
        .height = 0,
        .color_mode = ColorMode::RGBA,
        .tiles = {}
    };
    tad.tiles.reserve(filenames.size());
    for (const auto& [_, texture_size] : texture_sizes) {
        tad.width += texture_size(0);
        tad.height = std::max(tad.height, texture_size(1));
    }
    if (tad.width * tad.height > 4096 * 4096 * 20) {
        THROW_OR_ABORT("Atlas too large");
    }
    std::map<std::string, ManualUvTile> result;
    {
        int sum_width = 0;
        for (const auto& [filename, texture_size] : texture_sizes) {
            if (!result.insert({
                filename, {
                .position = {
                    (float)sum_width / (float)tad.width,
                    0.f},
                .size = texture_size.casted<float>()
                        / FixedArray<float, 2>{(float)tad.width, (float)tad.height}}}).second)
            {
                THROW_OR_ABORT("Detected duplicate atlas filename: \"" + filename + '"');
            }
            tad.tiles.push_back(ManualAtlasTileDescriptor{
                .left = sum_width,
                .bottom = 0,
                .filename = filename});
            sum_width += texture_size(0);
        }
    }
    linfo() << "Adding texture atlas of size " << tad.width << " x " << tad.height;
    // This call only works when a valid OpenGL context is acquired.
    // GLint max_texture_size;
    // CHK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size));
    // linfo() << "Max texture size: " << max_texture_size;
    add_manual_texture_atlas(name, tad);
    return result;
}

std::map<std::string, AutoUvTile> RenderingResources::generate_auto_texture_atlas(
    const std::string& name,
    const std::vector<std::string>& filenames,
    int mip_level_count,
    int size,
    AutoTextureAtlasDescriptor* atlas)
{
    std::map<std::string, FixedArray<int, 2>> packed_sizes;
    for (const auto& filename : filenames) {
        FixedArray<int, 2> image_size;
        if (preloaded_processed_texture_data_.contains({.filename = filename})) {
            const auto& img = preloaded_processed_texture_data_.get({.filename = filename});
            image_size = {img.width, img.height};
        } else if (preloaded_raw_texture_data_.contains(filename)) {
            auto info = ImageInfo::load(filename, &preloaded_raw_texture_data_.get(filename));
            image_size = {integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1))};
        } else if (preloaded_texture_dds_data_.contains(filename)) {
            auto info = ImageInfo::load(filename, &preloaded_texture_dds_data_.get(filename));
            image_size = {integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1))};
        } else {
            auto info = ImageInfo::load(filename, nullptr);
            image_size = {integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1))};
        }
        if (!packed_sizes.insert({filename, image_size}).second) {
            THROW_OR_ABORT("Found duplicate name \"" + filename + '"');
        }
    }
    FixedArray<int, 2> atlas_size_2d{size, size};
    auto packed_boxes = pack_boxes(packed_sizes, atlas_size_2d);
    AutoTextureAtlasDescriptor tad{
        .width = atlas_size_2d(0),
        .height = atlas_size_2d(1),
        .mip_level_count = mip_level_count,
        .color_mode = ColorMode::RGBA,
        .tiles = {}
    };
    if ((size_t)tad.width * (size_t)tad.height * packed_boxes.size() > 4096 * 4096 * 20) {
        THROW_OR_ABORT("Atlas too large");
    }
    std::map<std::string, AutoUvTile> result;
    tad.tiles.reserve(packed_boxes.size());
    for (const auto& [layer, nbs] : enumerate(packed_boxes)) {
        auto& tiles = tad.tiles.emplace_back();
        for (const auto& nb : nbs) {
            auto size_it = packed_sizes.find(nb.name);
            if (size_it == packed_sizes.end()) {
                THROW_OR_ABORT("Could not find texture with name \"" + nb.name + '"');
            }
            const auto& tile_size = size_it->second;
            if (!result.insert({
                nb.name,
                AutoUvTile{
                    .position = nb.bottom_left.casted<float>() / (atlas_size_2d.casted<float>() - 1.f),
                    .size = tile_size.casted<float>()
                            / FixedArray<float, 2>{(float)tad.width, (float)tad.height},
                    .layer = integral_cast<uint8_t>(layer)}}).second)
            {
                THROW_OR_ABORT("Detected duplicate atlas filename: \"" + nb.name + '"');
            }
            tiles.push_back(AutoAtlasTileDescriptor{
                .left = nb.bottom_left(0),
                .bottom = nb.bottom_left(1),
                .width = tile_size(0),
                .height = tile_size(1),
                .filename = nb.name});
        }
    }
    if (atlas != nullptr) {
        *atlas = tad;
    }
    linfo() << "Adding texture atlas of size " << tad.width << " x " << tad.height << " x " << tad.tiles.size();
    // This call only works when a valid OpenGL context is acquired.
    // GLint max_texture_size;
    // CHK(glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_texture_size));
    // linfo() << "Max texture size: " << max_texture_size;
    add_auto_texture_atlas(name, tad);
    return result;
}

BlendMapTexture RenderingResources::get_blend_map_texture(const std::string &name) const {
    LOG_FUNCTION("RenderingResources::get_blend_map_texture " + name);
    if (auto bit = blend_map_textures_.try_get(name); bit != nullptr) {
        return *bit;
    }
    std::shared_lock lock{mutex_};
    if (auto bit = blend_map_textures_.try_get(name); bit != nullptr) {
        return *bit;
    }
    if (auto tit = texture_descriptors_.try_get(name); tit != nullptr) {
        return BlendMapTexture{.texture_descriptor = *tit};
    }
    return BlendMapTexture{
        .texture_descriptor = {.color = {
            .filename = name,
            .mipmap_mode = MipmapMode::WITH_MIPMAPS}}};
}

void RenderingResources::set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt) {
    LOG_FUNCTION("RenderingResources::set_blend_map_texture " + name);
    blend_map_textures_.emplace(name, bmt);
}

const FixedArray<double, 4, 4>& RenderingResources::get_vp(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_vp " + name);
    auto it = vps_.try_get(name);
    if (it == nullptr) {
        THROW_OR_ABORT(
            "Could not find vp with name " + name + "."
            " Forgot to add a LightmapLogic for the light?"
            " Are dirtmaps enabled for the current scene?");
    }
    return *it;
}

void RenderingResources::set_vp(const std::string& name, const FixedArray<double, 4, 4>& vp) {
    LOG_FUNCTION("RenderingResources::set_vp " + name);
    vps_.set(name, vp);
}

float RenderingResources::get_offset(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    return offsets_.get(name);
}

void RenderingResources::set_offset(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_offset " + name);
    offsets_.set(name, value);
}

float RenderingResources::get_discreteness(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    return discreteness_.get(name);
}

void RenderingResources::set_discreteness(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_discreteness " + name);
    discreteness_.set(name, value);
}

float RenderingResources::get_scale(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_scale " + name);
    return scales_.get(name);
}

void RenderingResources::set_scale(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_scale " + name);
    scales_.set(name, value);
}

WrapMode RenderingResources::get_texture_wrap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_wrap " + name);
    return texture_wrap_.get(name);
}

void RenderingResources::set_texture_wrap(const std::string& name, WrapMode mode) {
    LOG_FUNCTION("RenderingResources::set_texture_wrap " + name);
    texture_wrap_.set(name, mode);
}

void RenderingResources::delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_vp " + name);
    if (vps_.erase(name) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            lwarn() << "Could not delete VP " << name;
        } else {
            THROW_OR_ABORT("Could not delete VP " + name);
        }
    }
}
void RenderingResources::delete_texture(const ColormapWithModifiers& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_texture " + name.filename);
    auto it = textures_.try_extract(name);
    if (it.empty()) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            lwarn() << "Could not delete texture " << name.filename;
        } else {
            verbose_abort("Could not delete texture " + name.filename);
        }
    } else if (it.mapped().owner == ResourceOwner::CONTAINER) {
        try_delete_texture(it.mapped().handle);
    }
}

ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& RenderingResources::render_programs() {
    std::shared_lock lock{mutex_};
    return render_programs_;
}

const std::string& RenderingResources::name() const {
    std::shared_lock lock{mutex_};
    return name_;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderingResources& r) {
    r.print(ostr);
    return ostr;
}

const LoadedFont& RenderingResources::get_font_texture(
    const std::string& ttf_filename,
    float font_height_pixels) const
{
    if (auto it = font_textures_.try_get({ttf_filename, font_height_pixels}); it != nullptr) {
        return *it;
    }
    std::scoped_lock lock{mutex_};
    if (auto it = font_textures_.try_get({ttf_filename, font_height_pixels}); it != nullptr) {
        return *it;
    }
    LoadedFont font;
    {
        const size_t TEXTURE_SIZE = 1024;
        std::vector<unsigned char> temp_bitmap(TEXTURE_SIZE * TEXTURE_SIZE);
        {
            std::vector<uint8_t> ttf_buffer = read_file_bytes(ttf_filename);
            // ASCII 32..126 is 95 glyphs
            font.cdata.resize(96);
            font.bottom_y = stbtt_BakeFontBitmap_get_y0(ttf_buffer.data(), 0, font_height_pixels, temp_bitmap.data(), TEXTURE_SIZE, TEXTURE_SIZE, 32, 96, font.cdata.data()); // no guarantee this fits!
            // can free ttf_buffer at this point
        }
        CHK(glGenTextures(1, &font.texture_handle));
        CHK(glBindTexture(GL_TEXTURE_2D, font.texture_handle));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data()));
        // can free temp_bitmap at this point
    }
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    return font_textures_.emplace({ttf_filename, font_height_pixels}, std::move(font));
}

void RenderingResources::save_to_file(
    const std::string& filename,
    const ColormapWithModifiers& color,
    TextureRole role) const
{
    if (!filename.ends_with(".png")) {
        THROW_OR_ABORT("Filename \"" + filename + "\" does not end with .png");
    }
    StbInfo img = get_texture_data(color, role, FlipMode::NONE);
    if (!stbi_write_png(
        filename.c_str(),
        img.width,
        img.height,
        img.nrChannels,
        img.data.get(),
        0))
    {
        THROW_OR_ABORT("Could not write \"" + filename + '"');
    }
}

void RenderingResources::insert_texture(
    const std::string& name,
    std::vector<uint8_t>&& data,
    TextureAlreadyExistsBehavior already_exists_behavior)
{
    LOG_FUNCTION("RenderingResources::set_texture " + name);
    std::scoped_lock lock{mutex_};

    if (preloaded_texture_dds_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "DDS-texture with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("DDS-texture with name \"" + name + "\" already exists");
    }
    if (preloaded_processed_texture_data_.contains({.filename = name})) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded processed non-DDS-texture with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded processed non-DDS-texture with name \"" + name + "\" already exists");
    }
    if (preloaded_raw_texture_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded raw non-DDS-texture with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded raw non-DDS-texture with name \"" + name + "\" already exists");
    }
    if (auto_atlas_tile_descriptors_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Auto texture atlas with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Auto texture atlas with name \"" + name + "\" already exists");
    }
    if (textures_.contains({.filename = name})) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Non-DDS-texture with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Non-DDS-texture with name \"" + name + "\" already exists");
    }
    auto extension = std::filesystem::path{name}.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        [](unsigned char c){ return std::tolower(c); });
    if ((extension == ".jpg") ||
        (extension == ".png") ||
        (extension == ".bmp"))
    {
        preloaded_raw_texture_data_.emplace(name, std::move(data));
    } else if (extension == ".dds") {
        preloaded_texture_dds_data_.emplace(name, std::move(data));
    } else {
        THROW_OR_ABORT("Unknown file extension: \"" + name + '"');
    }
}

void RenderingResources::initialize_non_dds_texture(const ColormapWithModifiers& color, TextureRole role) const
{
    auto generate_texture = [&color](
        const uint8_t* data,
        int width,
        int height,
        int nrChannels)
    {
        CHK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));  // https://stackoverflow.com/a/49126350/2292832
        {
#ifdef __ANDROID__
            auto nchannels = (size_t)nrChannels;
#else
            auto nchannels = (size_t)color.color_mode;
#endif
            CHK(glTexImage2D(
                GL_TEXTURE_2D,
                0,
                nchannels2internal_format(nchannels),
                width,
                height,
                0,
                nchannels2format((size_t)nrChannels),
                GL_UNSIGNED_BYTE,
                data));
        }
        // if (si.nrChannels == 4) {
        //     generate_rgba_mipmaps_inplace(si);
        // } else {
        //     CHK(glGenerateMipmap(GL_TEXTURE_2D));
        // }
        if (color.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
            CHK(glGenerateMipmap(GL_TEXTURE_2D));
        }
    };

    if (auto it = get_or_extract<EXTRACT_PROCESSED>(preloaded_processed_texture_data_, color); it != nullptr) {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Using preloaded texture: " << color;
        }
        generate_texture(it->data.get(), it->width, it->height, it->nrChannels);
    } else if (auto it = get_or_extract<EXTRACT_RAW>(preloaded_raw_texture_data_, color.filename); it != nullptr) {
        auto si = stb_load8(color.filename, FlipMode::NONE, it, IncorrectDatasizeBehavior::CONVERT);
        generate_texture(si.data.get(), si.width, si.height, si.nrChannels);
    } else {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Could not find preloaded texture: " << color;
        }
        auto si = get_texture_data(color, role, FlipMode::VERTICAL);
        generate_texture(si.data.get(), si.width, si.height, si.nrChannels);
    }
}

TextureSizeAndMipmaps RenderingResources::initialize_dds_texture(const ColormapWithModifiers& color) const
{
    auto data = get_or_extract<EXTRACT_RAW>(preloaded_texture_dds_data_, color.filename);

    nv_dds::CDDSImage image;
    {
        std::stringstream sstr;
        for (uint8_t c : *data) {
            sstr << c;
        }
        // Setting flipImage to false because it does not
        // work with image or mipmap sizes that are not a
        // multiple of 4.
        // Instead, the textures are now flipped using the GPU.
        image.load(sstr, false /* flipImage */);
    }

    if (image.get_num_mipmaps() == 0) {
        // if (descriptor.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
        //     CHK(glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP_SGIS, GL_TRUE));
        // }
        if (image.is_compressed()) {
            CHK(glCompressedTexImage2D(
                GL_TEXTURE_2D,
                0,
                image.get_format(),
                integral_cast<GLsizei>(image.get_width()),
                integral_cast<GLsizei>(image.get_height()),
                0,
                integral_cast<GLsizei>(image.get_size()),
                image));
        } else {
            CHK(glTexImage2D(
                GL_TEXTURE_2D,
                0,
                nchannels2internal_format(image.get_components()),
                integral_cast<GLsizei>(image.get_width()),
                integral_cast<GLsizei>(image.get_height()),
                0,
                image.get_format(),
                image.word_type(),
                image));
        }
        if (color.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
            CHK(glGenerateMipmap(GL_TEXTURE_2D));
        }
    } else {
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0));
        CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, integral_cast<GLint>(image.get_num_mipmaps())));

        if (image.is_compressed()) {
            CHK(glCompressedTexImage2D(
                GL_TEXTURE_2D,
                0,
                image.get_format(),
                integral_cast<GLsizei>(image.get_width()),
                integral_cast<GLsizei>(image.get_height()),
                0,
                integral_cast<GLsizei>(image.get_size()),
                image));

            for (unsigned int i = 0; i < image.get_num_mipmaps(); i++) {
                const nv_dds::CSurface& mipmap = image.get_mipmap(i);
                CHK(glCompressedTexImage2D(
                    GL_TEXTURE_2D,
                    integral_cast<GLint>(i + 1),
                    image.get_format(),
                    integral_cast<GLsizei>(mipmap.get_width()),
                    integral_cast<GLsizei>(mipmap.get_height()),
                    0,
                    integral_cast<GLsizei>(mipmap.get_size()),
                    mipmap));
            }
        } else {
            CHK(glTexImage2D(
                GL_TEXTURE_2D,
                0,
                nchannels2internal_format(image.get_components()),
                integral_cast<GLsizei>(image.get_width()),
                integral_cast<GLsizei>(image.get_height()),
                0,
                image.get_format(),
                image.word_type(),
                image));
            
            for (unsigned int i = 0; i < image.get_num_mipmaps(); i++) {
                const nv_dds::CSurface& mipmap = image.get_mipmap(i);
                CHK(glTexImage2D(
                    GL_TEXTURE_2D,
                    integral_cast<GLint>(i + 1),
                    nchannels2internal_format(image.get_components()),
                    integral_cast<GLsizei>(mipmap.get_width()),
                    integral_cast<GLsizei>(mipmap.get_height()),
                    0,
                    image.get_format(),
                    image.word_type(),
                    mipmap));
            }
        }
    }
    return {
        .width = integral_cast<GLsizei>(image.get_width()),
        .height = integral_cast<GLsizei>(image.get_height()),
        .nchannels = integral_cast<GLsizei>(image.get_components()),
        .mip_level_count = integral_cast<GLsizei>(image.get_num_mipmaps())};
}
