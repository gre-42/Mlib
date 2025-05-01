#include "Rendering_Resources.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geography/Heightmaps/Heightmap_To_Normalmap.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Geometry/Texture/Pack_Boxes.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
#include <Mlib/Images/Filters/Gaussian_Filter.hpp>
#include <Mlib/Images/Image_Info.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Images/Normalize.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <Mlib/Math/Is_Power_Of_Two.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Gl_Extensions.hpp>
#include <Mlib/Render/Instance_Handles/Bind_Texture_Guard.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Instance_Handles/Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Render_Guards.hpp>
#include <Mlib/Render/Instance_Handles/Texture.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Render/Render_Logics/Clear_Mode.hpp>
#include <Mlib/Render/Render_Logics/Fill_Pixel_Region_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Fill_With_Texture_Logic.hpp>
#include <Mlib/Render/Render_Logics/Resource_Update_Cycle.hpp>
#include <Mlib/Render/Render_Texture_Atlas.hpp>
#include <Mlib/Render/Render_To_Texture/Render_To_Texture_2D.hpp>
#include <Mlib/Render/Render_To_Texture/Render_To_Texture_2D_Array.hpp>
#include <Mlib/Render/Rendering_Context.hpp>
#include <Mlib/Render/Resource_Managers/Lazy_Texture.hpp>
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
#include <stb/stb_image_resize2.h>
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
#include <stb_cpp/stb_replace_color.hpp>
#include <stb_cpp/stb_saturate.hpp>
#include <stb_cpp/stb_set_alpha.hpp>
#include <stb_cpp/stb_transform.hpp>
#include <stb_cpp/stb_truetype_aligned.hpp>
#include <string>
#include <vector>

using namespace Mlib;

// Android makes use of the "deallocation_token_" and probably keeps the
// processed data for performance reasons.
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
    decltype(auto) operator * () {
        return node_.mapped();
    }
    decltype(auto) operator -> () {
        return &node_.mapped();
    }
    bool operator == (std::nullptr_t) const {
        return node_.empty();
    }
private:
    TNode node_;
};

static std::shared_ptr<StbInfo<uint8_t>> to_shared(StbInfo<uint8_t>&& i) {
    return std::make_shared<StbInfo<uint8_t>>(std::move(i));
}

template <class TContainer, class... TArgs>
auto& RenderingResources::add(TContainer& container, TArgs&&... args) const {
    assert_true(mutex_.is_owner());
    return container.add(std::forward<TArgs>(args)...);
}

template <class TContainer, class... TArgs>
auto& RenderingResources::add_font(TContainer& container, TArgs&&... args) const {
    // assert_true(font_mutex_.is_owner()); // No such method available in "SafeAtomicSharedMutex"
    return container.add(std::forward<TArgs>(args)...);
}

template <bool textract, class TContainer, class TKey>
auto RenderingResources::get_or_extract(TContainer& container, const TKey& key) const {
    assert_true(mutex_.is_owner());
    if constexpr (textract) {
        return ValueExtractor{ container.try_extract(key) };
    } else {
        return container.try_get(key);
    }
}

std::ostream& Mlib::operator << (std::ostream& ostr, const AutoAtlasTileDescriptor& aatd) {
    ostr <<
        "name: " << (const std::string&)aatd.name <<
        " position: (" << aatd.left << ", " << aatd.bottom <<
        ") size: (" << aatd.width << ", " << aatd.height << ')';
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const AutoTextureAtlasDescriptor& atad) {
    ostr <<
        "width: " << atad.width <<
        " height: " << atad.height <<
        " mip_level_count: " << atad.mip_level_count;
    for (const auto& [layer, tiles] : enumerate(atad.tiles)) {
        ostr << "  layer: " << layer << '\n';
        for (const auto& tile : tiles) {
            ostr << "  " << tile << '\n';
        }
    }
    return ostr;
}

std::ostream& Mlib::operator << (std::ostream& ostr, TextureType texture_type) {
    switch (texture_type) {
    case TextureType::TEXTURE_2D:
        return ostr << "texture_2d";
    case TextureType::TEXTURE_2D_ARRAY:
        return ostr << "texture_2d_array";
    case TextureType::TEXTURE_3D:
        return ostr << "texture_3d";
    case TextureType::TEXTURE_CUBE_MAP:
        return ostr << "texture_cube_map";
    }
    THROW_OR_ABORT("Unknown texture type: " + std::to_string((int)texture_type));
}

/**
 * From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
 */
static int log2(int n) {
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
    std::string touch_file = *color.filename + ".xpltd";
    auto source_color_mode = color.color_mode;
    // Color-selector
    bool has_color_selector =
        (color.selected_color_near != 0.f) ||
        (color.selected_color_far != INFINITY);
    if (has_color_selector) {
        if (color.color_mode != ColorMode::GRAYSCALE) {
            THROW_OR_ABORT("Color-selector requires grayscale");
        }
        source_color_mode = ColorMode::RGB;
    }
    // Height-to-normals
    if (color.height_to_normals) {
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("height_to_normals requires RGB");
        }
        source_color_mode = ColorMode::GRAYSCALE;
    }
    // Saturate
    if (color.saturate) {
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("saturate requires RGB");
        }
        source_color_mode = ColorMode::GRAYSCALE;
    }
    // "Color-selector" vs. "height-to-normals"
    if ((int)has_color_selector + (int)color.height_to_normals + (int)color.saturate > 1) {
        THROW_OR_ABORT("Only one out of \"color_selector\", \"height_to_normals\" and \"saturate\" may be specified");
    }
    if (any(source_color_mode & ColorMode::RGBA) &&
        color.alpha.empty() &&
        getenv_default_bool("EXTRAPOLATE_COLORS", false) &&
        !path_exists(touch_file))
    {
        linfo() << "Extrapolating RGBA image \"" << color << '"';
        auto img = StbImage4::load_from_file(*color.filename);
        float sigma = 3.f;
        size_t niterations = 1 + (size_t)((float)std::max(img.shape(0), img.shape(1)) / (sigma * 4));
        extrapolate_rgba_colors(
            img,
            sigma,
            niterations).save_to_file(*color.filename);
        auto ofstr = create_ofstream(touch_file);
        if (ofstr->fail()) {
            THROW_OR_ABORT("Could not create file \"" + touch_file + '"');
        }
    }
    StbInfo<uint8_t> si0;
    if (!color.alpha.empty()) {
        if (!any(source_color_mode & ColorMode::RGBA)) {
            THROW_OR_ABORT("Color mode not RGBA despite alpha texture: \"" + *color.filename + '"');
        }
        si0 = stb_load_texture(
            *color.filename, (int)max(ColorMode::RGB), flip_mode);
        if (si0.nrChannels != 3) {
            THROW_OR_ABORT("#channels not 3: \"" + *color.filename + '"');
        }
        auto si_alpha = stb_load_texture(
            color.alpha, (int)max(ColorMode::GRAYSCALE), flip_mode);
        if (si_alpha.nrChannels != 1) {
            THROW_OR_ABORT("#channels not 1: \"" + color.alpha + '"');
        }
        StbInfo<uint8_t> si0_rgb = std::move(si0);
        si0 = StbInfo<uint8_t>(si0_rgb.width, si0_rgb.height, 4);
        stb_set_alpha(
            si0_rgb.data(),
            si_alpha.data(),
            si0.data(),
            si0.width,
            si0.height,
            si_alpha.width,
            si_alpha.height);
    } else {
        si0 = stb_load_texture(
            *color.filename, (int)max(source_color_mode), flip_mode);
    }
    if (color.saturate) {
        if (si0.nrChannels != 1) {
            THROW_OR_ABORT("Saturate requires grayscale input");
        }
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("Saturate requires RGB output");
        }
        auto si1 = StbInfo<uint8_t>(si0.width, si0.height, 3);
        stb_saturate(
            si0.data(),
            si1.data(),
            si0.width,
            si0.height,
            si1.nrChannels);
        si0 = std::move(si1);
    }
    if (!color.average.empty()) {
        auto si1 = stb_load_texture(
            color.average, (int)max(source_color_mode), flip_mode);
        stb_average(
            si0.data(),
            si1.data(),
            si0.data(),
            si0.width,
            si0.height,
            si1.width,
            si1.height,
            si0.nrChannels,
            si1.nrChannels,
            si0.nrChannels);
    }
    if (color.desaturate != 0.f) {
        stb_desaturate(
            si0.data(),
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
        array_2_stb_image(m, si0.data());
    }
    if (!color.mean_color.all_equal(-1.f)) {
        if (!stb_colorize(
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            lwarn() << "alpha = 0: " << color << std::endl;
        }
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
            si0.data(),
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
            si0.data(),
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
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.lighten_top * 255.f).casted<short>().flat_begin(),
            (color.lighten_bottom * 255.f).casted<short>().flat_begin());
    }
    if (!color.alpha_blend.empty()) {
        auto si1 = stb_load_texture(
            color.alpha_blend, 4, flip_mode);
        stb_alpha_blend(
            si0.data(),
            si1.data(),
            si0.data(),
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
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            color.alpha_fac);
    }
    if (!color.color_to_replace.all_equal(-1.f)) {
        stb_replace_color(
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.color_to_replace * 255.f).casted<uint8_t>().flat_begin(),
            (color.replacement_color * 255.f).casted<uint8_t>().flat_begin(),
            (uint8_t)(std::round(color.replacement_tolerance * 255.f)));
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
        auto si1 = StbInfo<uint8_t>(si0.width, si0.height, 1);
        assert_isequal(si0.nrChannels, integral_cast<int>(CW::length(color.selected_color)));
        stb_generate_color_mask(
            si0.data(),
            si1.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (color.selected_color * 255.f).casted<short>().flat_begin(),
            (unsigned short)std::round(color.selected_color_near * 255.f),
            (unsigned short)std::round(color.selected_color_far * 255.f));
        si0 = std::move(si1);
    }
    if (color.edge_sigma != 0.f) {
        auto imf = multichannel_gaussian_filter_NWE(
            stb_image_2_array(si0).casted<float>() / 255.f,
            color.edge_sigma,
            (float)NAN,                 // boundary
            4.f,                        // truncate
            FilterExtension::PERIODIC);
        auto edges = 1.f - 2.f * abs(imf - 0.5f);
        array_2_stb_image((clipped(edges, 0.f, 1.f) * 255.f).casted<uint8_t>(), si0.data());
    }
    if ((color.times != 1.f) || (color.plus != 0.f)) {
        stb_transform(
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels,
            color.times,
            color.plus,
            color.abs);
    }
    if (color.invert) {
        stb_invert(
            si0.data(),
            si0.width,
            si0.height,
            si0.nrChannels);
    }
    if (!color.multiply.empty()) {
        auto si1 = stb_load_texture(
            color.multiply, (int)max(source_color_mode), flip_mode);
        stb_multiply_color(
            si0.data(),
            si1.data(),
            si0.data(),
            si0.width,
            si0.height,
            si1.width,
            si1.height,
            si0.nrChannels,
            si1.nrChannels,
            si0.nrChannels);
    }
    if (color.height_to_normals) {
        if (si0.nrChannels != 1) {
            THROW_OR_ABORT("Height-to-normals requires grayscale input");
        }
        if (color.color_mode != ColorMode::RGB) {
            THROW_OR_ABORT("Height-to-normals requires RGB output");
        }
        // Number of intensity differences between neighboring pixels that will kind of
        // saturate the normal (kind of: the differences are compared to "1" during normal
        // computation, so the normals will not really saturate).
        const auto intensity_range = 10.f;
        auto heightmap = stb_image_2_array(si0)[0];
        si0 = StbInfo<uint8_t>(si0.width, si0.height, 3);
        auto nm_transposed = clipped(heightmap_to_normalmap(heightmap.casted<float>(), intensity_range) * 127.5f + 127.5f, 0.f, 255.f).casted<uint8_t>();
        auto nm = Array<uint8_t>{ nm_transposed.shape() };
        nm[0] = nm_transposed[1];
        nm[1] = nm_transposed[0];
        nm[2] = nm_transposed[2];
        array_2_stb_image(nm, si0.data());
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
            opacity += double(si.data()[(r * si.width + c) * si.nrChannels + 3]) / 255.;
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
            THROW_OR_ABORT("Colormode undefined in color texture \"" + *color.filename + '"');
        }
    } else if (role == TextureRole::SPECULAR) {
        if (!any(color.color_mode & ColorMode::RGB)) {
            THROW_OR_ABORT("Colormode not RGB in specularmap: \"" + *color.filename + '"');
        }
    } else if (role == TextureRole::NORMAL) {
        if (!any(color.color_mode & ColorMode::RGB)) {
            THROW_OR_ABORT("Colormode not RGB in normalmap: \"" + *color.filename + '"');
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
//     RgbaDownsampler rds{si.data(), si.width, si.height};
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
    for (const auto& [n, _] : texture_descriptors_) {
        ostr << indent << "  " << *n << '\n';
    }
    ostr << indent << "Blend map textures\n";
    for (const auto& [n, _] : blend_map_textures_) {
        ostr << indent << "  " << *n << '\n';
    }
    ostr << indent << "Textures\n";
    for (const auto& [n, _] : textures_) {
        ostr << indent << "  " << n << '\n';
    }
    ostr << indent << "Aliases\n";
    for (const auto& [n, _] : aliases_) {
        ostr << indent << "  " << *n << '\n';
    }
    ostr << indent << "vps\n";
    for (const auto& [n, _] : vps_) {
        ostr << indent << "  " << *n << '\n';
    }
    ostr << indent << "Discreteness\n";
    for (const auto& [n, _] : discreteness_) {
        ostr << indent << "  " << *n << '\n';
    }
    ostr << indent << "DDS\n";
    for (const auto& [n, _] : preloaded_texture_dds_data_) {
        ostr << indent << "  " << n << '\n';
    }
}

RenderingResources::RenderingResources(
    std::string name,
    unsigned int max_anisotropic_filtering_level)
    : preloaded_processed_texture_data_{
        "Preloaded processed texture data",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , preloaded_processed_texture_array_data_{
        "Preloaded processed texture array data",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , preloaded_raw_texture_data_{
        "Preloaded raw texture data",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , preloaded_texture_dds_data_{
        "Preloaded texture DDS data",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , texture_types_{
        "Texture types",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , texture_descriptors_{
        "Texture descriptor",
        [](const VariableAndHash<std::string>& e) { return *e; } }
    , textures_{ "Texture", [](const ColormapWithModifiers& e) { return *e.filename; } }
    , texture_sizes_{ "Texture size" }
    , manual_atlas_tile_descriptors_{ "Manual atlas tile descriptor" }
    , auto_atlas_tile_descriptors_{
        "Auto atlas tile descriptor",
        [](const ColormapWithModifiers& e) { return *e.filename; } }
    , cubemap_descriptors_{ "Cubemap descriptor" }
    , charsets_{ "Charset" }
    , font_textures_{ "Font", [](const auto& e) { return e.ttf_filename; } }
    , aliases_{ "Alias" }
    , vps_{ "VP" }
    , offsets_{ "Offset" }
    , discreteness_{ "Discreteness" }
    , scales_{ "Scale" }
    , blend_map_textures_{ "Blend-map texture" }
    , render_programs_{ "Render program", [](const RenderProgramIdentifier& e) { return "<RPI>"; } }
    , name_{ std::move(name) }
    , max_anisotropic_filtering_level_{ max_anisotropic_filtering_level }
    , preloader_background_loop_{ "Preload_BG" }
    , deallocation_token_{ render_deallocator.insert([this]() {
        for (const auto& [d, _] : textures_) {
            append_render_allocator([this, w = std::weak_ptr{ lifetime_indicator_ }, d = d]() {
                if (auto s = w.lock()) {
                    preload(d, TextureRole::TRUSTED);
                }
            });
        }
        {
            std::shared_lock lock{ mutex_ };
            for (const auto& state : set_textures_lazy_) {
                state->notify_deactivated();
                append_render_allocator(state->generate_activator());
            }
        }
        deallocate();
    }) }
    , lifetime_indicator_{ std::make_shared<int>(42) }
{}

RenderingResources::~RenderingResources() {
    deallocate();
}

void RenderingResources::deallocate() {
    std::scoped_lock lock{ mutex_ };
    render_programs_.clear();
    textures_.clear();
    texture_types_.clear();
    font_textures_.clear();
    texture_sizes_.clear();
}

void RenderingResources::preload(const TextureDescriptor& descriptor) const {
    auto dit = texture_descriptors_.try_get(descriptor.color.filename);
    const TextureDescriptor& desc = dit != nullptr
        ? *dit
        : descriptor;
    if (!desc.color.filename->empty()) {
        preload(desc.color, TextureRole::COLOR);
    }
    if (!desc.specular.filename->empty()) {
        preload(desc.specular, TextureRole::SPECULAR);
    }
    if (!desc.normal.filename->empty()) {
        preload(desc.normal, TextureRole::NORMAL);
        // auto ditn = texture_descriptors_.try_get(desc.normal.filename);
        // if (ditn != nullptr) {
        //     preload(ditn->color, TextureRole::NORMAL);
        // } else {
        //     preload(desc.normal, TextureRole::NORMAL);
        // }
    }
}

void RenderingResources::preload(const ColormapWithModifiers& color, TextureRole role) const {
    LOG_FUNCTION("RenderingResources::preload, color=" + color);
    if (color.filename->empty()) {
        THROW_OR_ABORT("Attempt to preload empty texture");
    }
    {
        std::shared_lock lock{ mutex_ };
        if (textures_.contains(color)) {
            return;
        }
    }
    std::scoped_lock lock{ mutex_ };
    if (textures_.contains(color)) {
        return;
    }
    if (ContextQuery::is_initialized()) {
        get_texture(color, role, CallerType::PRELOAD);
    } else {
        check_color_mode(color, role);
        if (!preloaded_processed_texture_data_.contains(color) &&
            !preloaded_processed_texture_array_data_.contains(color) &&
            !preloaded_raw_texture_data_.contains(color) &&
            !preloaded_texture_dds_data_.contains(color) &&
            !auto_atlas_tile_descriptors_.contains(color))
        {
            if (manual_atlas_tile_descriptors_.contains(color.filename)) {
                auto data = get_texture_array_data(color, role, FlipMode::VERTICAL);
                add(preloaded_processed_texture_array_data_, color, std::move(data));
            } else {
                auto data = get_texture_data(color, role, FlipMode::VERTICAL);
                add(preloaded_processed_texture_data_, color, std::move(data));
            }
            if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                linfo() << this << " Preloaded texture: " << color;
            }
        }
        append_render_allocator([this, color, role]() { get_texture(color, role, CallerType::PRELOAD); });
    }
}

std::shared_ptr<ITextureHandle> RenderingResources::get_texture_lazy(
    const ColormapWithModifiers& name,
    TextureRole role) const
{
    return std::make_shared<LazyTexture>(*this, name, role);
}

bool RenderingResources::texture_is_loaded_and_try_preload(
    const ColormapWithModifiers& color,
    TextureRole role) const
{
    if (texture_is_loaded_unsafe(color)) {
        return true;
    }
    if (!preloader_background_loop_.done()) {
        return false;
    }
    preloader_background_loop_.try_run([this, color, role](){
        preload(color, role);
    });
    return false;
}

bool RenderingResources::texture_is_loaded_unsafe(const ColormapWithModifiers& name) const {
    if (preloaded_texture_dds_data_.contains(name)) {
        return true;
    }
    if (preloaded_processed_texture_data_.contains(name)) {
        return true;
    }
    if (preloaded_processed_texture_array_data_.contains(name)) {
        return true;
    }
    if (preloaded_raw_texture_data_.contains(name)) {
        return true;
    }
    if (textures_.contains(name)) {
        return true;
    }
    if (auto_atlas_tile_descriptors_.contains(name)) {
        THROW_OR_ABORT("Attempted lazy access to auto texture atlas");
    }
    return false;
}

bool RenderingResources::contains_texture(const ColormapWithModifiers& name) const {
    std::shared_lock lock{ mutex_ };
    return textures_.contains(name) || auto_atlas_tile_descriptors_.contains(name);
}

TextureType RenderingResources::texture_type(
    const ColormapWithModifiers& name,
    TextureRole role) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return texture_type(colormap(name), TextureRole::COLOR);
    }
    if (auto_atlas_tile_descriptors_.contains(name)) {
        return TextureType::TEXTURE_2D_ARRAY;
    }
    if (auto it = manual_atlas_tile_descriptors_.try_get(name.filename); it != nullptr) {
        return it->nlayers == 1
            ? TextureType::TEXTURE_2D
            : it->depth_interpolation == InterpolationMode::NEAREST
                ? TextureType::TEXTURE_2D_ARRAY
                : TextureType::TEXTURE_3D;
    }
    if (auto it = preloaded_processed_texture_array_data_.try_get(name); it != nullptr) {
        return it->size() == 1 ? TextureType::TEXTURE_2D : TextureType::TEXTURE_2D_ARRAY;
    }
    if (auto it = preloaded_raw_texture_data_.try_get(name); it != nullptr) {
        return TextureType::TEXTURE_2D;
    }
    if (preloaded_processed_texture_data_.contains(name)) {
        return TextureType::TEXTURE_2D;
    }
    if (cubemap_descriptors_.contains(name.filename)) {
        return TextureType::TEXTURE_CUBE_MAP;
    }
    if (auto it = texture_types_.try_get(name); it != nullptr) {
        return *it;
    }
    if (path_exists(*name.filename)) {
        return TextureType::TEXTURE_2D;
    }
    THROW_OR_ABORT("Could not find texture:\n" + (std::stringstream() << name).str());
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
        std::vector<std::shared_ptr<StbInfo<uint8_t>>> sis;
        if (manual_atlas_tile_descriptors_.contains(color.filename)) {
            sis = get_texture_array_data(color, role, FlipMode::VERTICAL);
        } else {
            sis.push_back(get_texture_data(color, role, FlipMode::VERTICAL));
        }
        if (sis.empty()) {
            THROW_OR_ABORT("Texture array \"" + *color.filename + "\" has no layers");
        }
        if (sis.size() != 1) {
            lwarn() << "Texture array \"" << *color.filename << "\" has more than one layer. Only saving the first one.";
        }
        const auto& si = *sis[0];
        if (!default_filename.ends_with(".png")) {
            THROW_OR_ABORT("Filename \"" + default_filename + "\" does not end with .png");
        }
        if (!stbi_write_png(
            default_filename.c_str(),
            si.width,
            si.height,
            si.nrChannels,
            si.data(),
            0))
        {
            THROW_OR_ABORT("Could not save to file \"" + default_filename + '"');
        }
        return default_filename;
    } else {
        return *color.filename;
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

std::shared_ptr<ITextureHandle> TextureSizeAndMipmaps::flipped_vertically(float aniso) const {
    FillWithTextureLogic logic{
        handle,
        CullFaceMode::CULL,
        ContinuousBlendMode::NONE,
        vertically_flipped_quad_vertices };
    GLuint texture = render_to_texture_2d(
        width,
        height,
        // https://stackoverflow.com/questions/9572414/how-many-mipmaps-does-a-texture-have-in-opengl
        (mip_level_count == 0)
        ? integral_cast<GLsizei>(log2(std::max(width, height)))
        : mip_level_count,
        aniso,
        nchannels2sized_internal_format(integral_cast<size_t>(nchannels)),
        [&logic](GLsizei width, GLsizei height)
        {
            ViewportGuard vg{
                0.f,
                0.f,
                integral_to_float<float>(width),
                integral_to_float<float>(height)};
            logic.render(ClearMode::COLOR);
        });
    return std::make_shared<Texture>(
        texture,
        nchannels2format(integral_cast<size_t>(nchannels)),
        mip_level_count != 0,
        wrap_s,
        wrap_t);
}

std::shared_ptr<ITextureHandle> RenderingResources::get_texture(
    const VariableAndHash<std::string>& name,
    CallerType caller_type) const
{
    const auto& desc = get_texture_descriptor(name);
    return get_texture(desc.color, TextureRole::COLOR, caller_type);
}

std::shared_ptr<ITextureHandle> RenderingResources::get_texture(
    const ColormapWithModifiers& color,
    TextureRole role,
    CallerType caller_type) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture(colormap(color), TextureRole::COLOR, caller_type);
    }
    check_color_mode(color, role);
    LOG_FUNCTION("RenderingResources::get_texture " + color.filename);
    {
        std::shared_lock lock{ mutex_ };
        if (auto it = textures_.try_get(color); it != nullptr) {
            return it->handle;
        }
    }
    std::scoped_lock lock{ mutex_ };
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

    auto make_shared_texture = [&](GLuint handle) {
        return std::make_shared<Texture>(handle, color.color_mode, color.mipmap_mode, color.wrap_modes);
        };
    std::shared_ptr<ITextureHandle> texture;

    if (auto aptr = get_or_extract<EXTRACT_PROCESSED>(auto_atlas_tile_descriptors_, color); aptr != nullptr) {
        if (caller_type != CallerType::PRELOAD) {
            THROW_OR_ABORT("Texture source is not preload for texture \"" + *color.filename + '"');
        }
        texture = make_shared_texture(render_to_texture_2d_array(
            aptr->width,
            aptr->height,
            integral_cast<GLsizei>(aptr->tiles.size()),
            aptr->mip_level_count,
            aniso,
            nchannels2sized_internal_format(max(color.color_mode)),
            [this, &aptr](GLsizei width, GLsizei height, GLsizei layer)
            {
                render_texture_atlas(
                    *const_cast<RenderingResources*>(this),
                    aptr->tiles.at(integral_cast<size_t>(layer)),
                    integral_to_float<float>(width) / integral_to_float<float>(aptr->width),
                    integral_to_float<float>(height) / integral_to_float<float>(aptr->height));
            }));
    } else {
        if (preloaded_texture_dds_data_.contains(color)) {
            texture = initialize_dds_texture(color, aniso);
        } else if (cubemap_descriptors_.contains(color.filename)) {
            texture = make_shared_texture(get_cubemap_unsafe(color.filename));
        } else {
            auto t = initialize_non_dds_texture(color, role, aniso);
            texture = make_shared_texture(t.first);
            add(texture_types_, color, t.second);
        }
    }

    return add(textures_, color, std::move(texture)).handle;
}

GLuint RenderingResources::get_cubemap_unsafe(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_cubemap " + *name);
    const auto& cd = cubemap_descriptors_.get(name);
    if (cd.filenames.size() != 6) {
        THROW_OR_ABORT("Cubemap does not have 6 filenames");
    }
    GLuint textureID;
    CHK(glGenTextures(1, &textureID));
    CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    for (const auto& [i, color] : enumerate(cd.filenames)) {
        auto dit = texture_descriptors_.try_get(color);
        ColormapWithModifiers desc = dit != nullptr
            ? dit->color
            : ColormapWithModifiers{
                .filename = color,
                .color_mode = ColorMode::RGB}.compute_hash();

        auto info = get_texture_data(desc, TextureRole::COLOR, FlipMode::NONE);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/a/49126350/2292832
        CHK(glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + integral_cast<GLuint>(i),
            0,
            GL_RGB,
            info->width,
            info->height,
            0,
            GL_RGB,
            GL_UNSIGNED_BYTE,
            info->data()));
    }
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    CHK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));
    CHK(glGenerateMipmap(GL_TEXTURE_CUBE_MAP));

    return textureID;
}

void RenderingResources::add_texture(
    const ColormapWithModifiers& name,
    std::shared_ptr<ITextureHandle> id,
    const TextureSize* texture_size)
{
    LOG_FUNCTION("RenderingResources::set_texture " + *name.filename);
    std::scoped_lock lock{ mutex_ };
    add(textures_, name, TextureHandleAndOwner{ .handle = id });
    if (texture_size != nullptr) {
        add(texture_sizes_, name.filename, *texture_size);
    }
}

void RenderingResources::set_texture(
    const ColormapWithModifiers& name,
    std::shared_ptr<ITextureHandle> id,
    const TextureSize* texture_size)
{
    LOG_FUNCTION("RenderingResources::set_texture " + *name.filename);
    std::scoped_lock lock{ mutex_ };
    if (auto v = textures_.try_get(name); v != nullptr) {
        v->handle = std::move(id);
    } else {
        add(textures_, name, TextureHandleAndOwner{ .handle = std::move(id) });
    }
    if (texture_size != nullptr) {
        texture_sizes_.insert_or_assign(name.filename, *texture_size);
    }
}

void RenderingResources::set_textures_lazy(std::function<void()> func)
{
    auto state = std::make_shared<ActivationState>(std::move(func));
    {
        std::scoped_lock lock{ mutex_ };
        set_textures_lazy_.push_back(state);
    }
    append_render_allocator(state->generate_activator());
}

void RenderingResources::add_texture_descriptor(const VariableAndHash<std::string>& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + *name);
    if (descriptor.color.color_mode == ColorMode::UNDEFINED) {
        THROW_OR_ABORT("Colormode undefined color texture: \"" + *descriptor.color.filename + '"');
    }
    if (!descriptor.specular.filename->empty() && !any(descriptor.specular.color_mode & ColorMode::RGB)) {
        THROW_OR_ABORT("Colormode not RGB in specularmap: \"" + *descriptor.specular.filename + '"');
    }
    if (!descriptor.normal.filename->empty() && !any(descriptor.normal.color_mode & ColorMode::RGB)) {
        THROW_OR_ABORT("Colormode not RGB in normalmap: \"" + *descriptor.normal.filename + '"');
    }
    std::scoped_lock lock{ mutex_ };
    add(texture_descriptors_, name, descriptor);
}

bool RenderingResources::contains_texture_descriptor(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_descriptor " + *name);
    return texture_descriptors_.contains(name);
}

TextureDescriptor RenderingResources::get_texture_descriptor(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_descriptor " + *name);
    return texture_descriptors_.get(name);
}

void RenderingResources::add_manual_texture_atlas(
    const VariableAndHash<std::string>& name,
    const ManualTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_manual_texture_atlas " + *name);
    std::scoped_lock lock{ mutex_ };
    add(manual_atlas_tile_descriptors_, name, texture_atlas_descriptor);
}

void RenderingResources::add_auto_texture_atlas(
    const ColormapWithModifiers& name,
    const AutoTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_auto_texture_atlas " + *name);
    std::scoped_lock lock{ mutex_ };
    add(auto_atlas_tile_descriptors_, name, texture_atlas_descriptor);
    append_render_allocator([this, name]() { preload(name, TextureRole::COLOR); });
}

void RenderingResources::add_cubemap(const VariableAndHash<std::string>& name, const std::vector<VariableAndHash<std::string>>& filenames) {
    LOG_FUNCTION("RenderingResources::add_cubemap " + *name);
    std::scoped_lock lock{ mutex_ };
    if (texture_descriptors_.contains(name)) {
        THROW_OR_ABORT("Texture descriptor with name \"" + *name + "\" already exists");
    }
    add(cubemap_descriptors_, name, CubemapDescriptor{.filenames = filenames});
}

std::vector<std::shared_ptr<StbInfo<uint8_t>>> RenderingResources::get_texture_array_data(
    const ColormapWithModifiers& color,
    TextureRole role,
    FlipMode flip_mode) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture_array_data(colormap(color), TextureRole::COLOR, flip_mode);
    }
    check_color_mode(color, role);
    if (auto it = manual_atlas_tile_descriptors_.try_get(color.filename); it != nullptr) {
        std::vector<std::shared_ptr<StbInfo<uint8_t>>> sis;
        sis.reserve(it->nlayers);
        for (size_t i = 0; i < it->nlayers; ++i) {
            sis.push_back(to_shared(StbInfo<uint8_t>(it->width, it->height, (int)max(it->color_mode))));
        }
        UnorderedMap<ColormapWithModifiers, std::shared_ptr<StbInfo<uint8_t>>> source_images;
        std::vector<AtlasTile> atlas_tiles;
        atlas_tiles.reserve(it->tiles.size());
        for (const auto& [source, target] : it->tiles) {
            const auto* si = source_images.try_get(source.name);
            if (si == nullptr) {
                auto it = source_images.try_emplace(source.name, get_texture_data(source.name, role, flip_mode));
                if (!it.second) {
                    verbose_abort("Could not cache \"" + *source.name.filename + '"');
                }
                si = &it.first->second;
            }
            atlas_tiles.push_back(AtlasTile{
                .source = {
                    .left = source.left,
                    .bottom = source.bottom,
                    .width = source.width,
                    .height = source.height,
                    .image = **si
                },
                .target = {
                    .left = target.left,
                    .bottom = target.bottom,
                    .layer = target.layer
                } });
        }
        build_image_atlas(sis, atlas_tiles);
        return sis;
    }
    THROW_OR_ABORT("Unknown texture array: \"" + *color.filename + '"');
}

std::shared_ptr<StbInfo<uint8_t>> RenderingResources::get_texture_data(
    const ColormapWithModifiers& color,
    TextureRole role,
    FlipMode flip_mode) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return get_texture_data(colormap(color), TextureRole::COLOR, flip_mode);
    }
    check_color_mode(color, role);
    if (auto it = preloaded_texture_dds_data_.try_get(color); it != nullptr) {
        auto info = ImageInfo::load(*color.filename, &it->data);
        auto fb = std::make_shared<FrameBuffer>(CURRENT_SOURCE_LOCATION);
        fb->configure(FrameBufferConfig{
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
            get_texture(color),
            CullFaceMode::CULL,
            ContinuousBlendMode::NONE};
        {
            RenderToFrameBufferGuard rfg{ fb };
            logic.render(ClearMode::COLOR);
        }
        return to_shared(fb->color_to_stb_image(4));
    }
    if (auto it = preloaded_processed_texture_data_.try_get(color); it != nullptr) {
        return *it;
    }
    if (auto it = preloaded_raw_texture_data_.try_get(color); it != nullptr) {
        return to_shared(stb_load8(*color.filename, FlipMode::NONE, &it->data, IncorrectDatasizeBehavior::CONVERT));
    }
    auto si = stb_load_and_transform_texture(color, flip_mode);
    if (any(color.color_mode & ColorMode::RGB) &&
        (si.nrChannels == 4) &&
        getenv_default_bool("CHECK_OPACITY", false))
    {
        double opacity = mean_opacity(si);
        if (opacity < 0.8) {
            lwarn() << color << ": Opacity is only " << opacity;
        }
    }
    return to_shared(std::move(si));
}

std::map<ColormapWithModifiers, ManualUvTile> RenderingResources::generate_manual_texture_atlas(
    const VariableAndHash<std::string>& name,
    const std::vector<ColormapWithModifiers>& filenames)
{
    std::map<ColormapWithModifiers, FixedArray<int, 2>> texture_sizes;
    for (const auto& filename : filenames) {
        int x;
        int y;
        int comp;
        if (stbi_info(filename.filename->c_str(), &x, &y, &comp) == 0) {
            THROW_OR_ABORT("Could not read size information from file \"" + *filename.filename + '"');
        }
        if (!texture_sizes.try_emplace(filename, x, y).second) {
            THROW_OR_ABORT("Detected duplicate texture: \"" + *filename.filename + '"');
        }
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
    std::map<ColormapWithModifiers, ManualUvTile> result;
    {
        int sum_width = 0;
        for (const auto& [name, texture_size] : texture_sizes) {
            auto size = texture_size.casted<float>() /
                FixedArray<int, 2>{tad.width, tad.height}.casted<float>();
            auto position = FixedArray<float, 2>{
                (float)sum_width / (float)tad.width,
                0.f
            };
            if (!result.try_emplace(name, ManualUvTile{ .position = position, .size = size }).second) {
                THROW_OR_ABORT("Detected duplicate atlas filename: \"" + *name.filename + '"');
            }
            tad.tiles.push_back(ManualAtlasTileDescriptor{
                .source = {
                    .left = 0,
                    .bottom = 0,
                    .width = texture_size(0),
                    .height = texture_size(1),
                    .name = name
                },
                .target = {
                    .left = sum_width,
                    .bottom = 0,
                    .layer = 0
                } });
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

FixedArray<int, 2> RenderingResources::texture_size(const ColormapWithModifiers& name) const {
    FixedArray<int, 2> image_size = uninitialized;
    if (auto* img = preloaded_processed_texture_data_.try_get(name); img != nullptr) {
        image_size = { (*img)->width, (*img)->height };
    } else if (auto* img = preloaded_raw_texture_data_.try_get(name); img != nullptr) {
        auto info = ImageInfo::load(*name.filename, &img->data);
        image_size = { integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1)) };
    } else if (auto* img = preloaded_texture_dds_data_.try_get(name); img != nullptr) {
        auto info = ImageInfo::load(*name.filename, &img->data);
        image_size = { integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1)) };
    } else if (auto* img = texture_sizes_.try_get(name.filename); img != nullptr) {
        image_size = { img->width, img->height };
    } else {
        auto info = ImageInfo::load(*name.filename, nullptr);
        image_size = { integral_cast<int>(info.size(0)), integral_cast<int>(info.size(1)) };
    }
    return image_size;
}

std::unordered_map<VariableAndHash<std::string>, AutoUvTile> RenderingResources::generate_auto_texture_atlas(
    const ColormapWithModifiers& name,
    const std::vector<ColormapWithModifiers>& filenames,
    int mip_level_count,
    int size,
    AutoTextureAtlasDescriptor* atlas)
{
    std::unordered_map<VariableAndHash<std::string>, ColormapWithModifiers> colormaps;
    std::unordered_map<VariableAndHash<std::string>, FixedArray<int, 2>> packed_sizes;
    for (const auto& colormap : filenames) {
        auto image_size = texture_size(colormap);
        if (!packed_sizes.try_emplace(colormap.filename, image_size).second) {
            THROW_OR_ABORT("Found duplicate name \"" + (const std::string&)colormap + '"');
        }
        if (!colormaps.try_emplace(colormap.filename, colormap).second) {
            THROW_OR_ABORT("Found duplicate name \"" + (const std::string&)colormap + '"');
        }
    }
    FixedArray<int, 2> atlas_size_2d{ size, size };
    auto packed_boxes = pack_boxes(packed_sizes, atlas_size_2d);
    AutoTextureAtlasDescriptor tad{
        .width = atlas_size_2d(0),
        .height = atlas_size_2d(1),
        .mip_level_count = mip_level_count,
        .tiles = {}
    };
    if ((size_t)tad.width * (size_t)tad.height * packed_boxes.size() > (size_t)4096 * (size_t)4096 * (size_t)20) {
        THROW_OR_ABORT("Atlas too large");
    }
    std::unordered_map<VariableAndHash<std::string>, AutoUvTile> result;
    tad.tiles.reserve(packed_boxes.size());
    for (const auto& [layer, nbs] : enumerate(packed_boxes)) {
        auto& tiles = tad.tiles.emplace_back();
        for (const auto& nb : nbs) {
            auto size_it = packed_sizes.find(nb.name);
            if (size_it == packed_sizes.end()) {
                THROW_OR_ABORT("Could not find texture with name \"" + *nb.name + '"');
            }
            const auto& tile_size = size_it->second;
            if (!result.try_emplace(
                nb.name,
                AutoUvTile{
                    .position = nb.bottom_left.casted<float>() / (atlas_size_2d.casted<float>() - 1.f),
                    .size = tile_size.casted<float>()
                            / FixedArray<float, 2>{(float)tad.width, (float)tad.height},
                    .layer = integral_cast<uint8_t>(layer)}).second)
            {
                THROW_OR_ABORT("Detected duplicate atlas filename: \"" + *nb.name + '"');
            }
            tiles.push_back(AutoAtlasTileDescriptor{
                .left = nb.bottom_left(0),
                .bottom = nb.bottom_left(1),
                .width = tile_size(0),
                .height = tile_size(1),
                .name = colormaps.at(nb.name) });
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

BlendMapTexture RenderingResources::get_blend_map_texture(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_blend_map_texture " + *name);
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
    if (auto mit = manual_atlas_tile_descriptors_.try_get(name); mit != nullptr) {
        return BlendMapTexture{
            .texture_descriptor = {.color = ColormapWithModifiers{
                .filename = name,
                .color_mode = mit->color_mode,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS,
                .depth_interpolation = mit->depth_interpolation}.compute_hash()}};
    }
    {
        return BlendMapTexture{
            .texture_descriptor = {.color = ColormapWithModifiers{
                .filename = name,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS}.compute_hash()} };
    }
}

void RenderingResources::set_blend_map_texture(const VariableAndHash<std::string>& name, const BlendMapTexture& bmt) {
    LOG_FUNCTION("RenderingResources::set_blend_map_texture " + *name);
    std::scoped_lock lock{ mutex_ };
    add(blend_map_textures_, name, bmt);
}

void RenderingResources::set_alias(VariableAndHash<std::string> alias, VariableAndHash<std::string> name) {
    std::scoped_lock lock{ mutex_ };
    add(aliases_, std::move(alias), std::move(name));
}

VariableAndHash<std::string> RenderingResources::get_alias(const VariableAndHash<std::string>& alias) const {
    return aliases_.get(alias);
}

bool RenderingResources::contains_alias(const VariableAndHash<std::string>& alias) const {
    return aliases_.contains(alias);
}

const FixedArray<ScenePos, 4, 4>& RenderingResources::get_vp(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_vp " + *name);
    auto it = vps_.try_get(name);
    if (it == nullptr) {
        THROW_OR_ABORT(
            "Could not find vp with name " + *name + "."
            " Forgot to add a LightmapLogic for the light?"
            " Are dirtmaps enabled for the current scene?"
            " Are primary/secondary rendering resources mixed up?");
    }
    return *it;
}

void RenderingResources::set_vp(const VariableAndHash<std::string>& name, const FixedArray<ScenePos, 4, 4>& vp) {
    LOG_FUNCTION("RenderingResources::set_vp " + *name);
    vps_.insert_or_assign(name, vp);
}

float RenderingResources::get_offset(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + *name);
    return offsets_.get(name);
}

void RenderingResources::set_offset(const VariableAndHash<std::string>& name, float value) {
    LOG_FUNCTION("RenderingResources::set_offset " + *name);
    offsets_.insert_or_assign(name, value);
}

float RenderingResources::get_discreteness(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + *name);
    return discreteness_.get(name);
}

void RenderingResources::set_discreteness(const VariableAndHash<std::string>& name, float value) {
    LOG_FUNCTION("RenderingResources::set_discreteness " + *name);
    discreteness_.insert_or_assign(name, value);
}

float RenderingResources::get_scale(const VariableAndHash<std::string>& name) const {
    LOG_FUNCTION("RenderingResources::get_scale " + *name);
    return scales_.get(name);
}

void RenderingResources::set_scale(const VariableAndHash<std::string>& name, float value) {
    LOG_FUNCTION("RenderingResources::set_scale " + *name);
    scales_.insert_or_assign(name, value);
}

void RenderingResources::delete_vp(const VariableAndHash<std::string>& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_vp " + *name);
    if (vps_.erase(name) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            lwarn() << "Could not delete VP " << *name;
        } else {
            THROW_OR_ABORT("Could not delete VP " + *name);
        }
    }
}

void RenderingResources::delete_texture(const ColormapWithModifiers& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_texture " + *name.filename);
    auto it = textures_.try_extract(name);
    if (it.empty()) {
        switch (deletion_failure_mode) {
        case DeletionFailureMode::IGNORE:
            // Do nothing
            return;
        case DeletionFailureMode::WARN:
            lwarn() << "Could not delete texture " << *name.filename;
            return;
        case DeletionFailureMode::ABORT:
            verbose_abort("Could not delete texture " + *name.filename);
        }
        verbose_abort("Unknown deletion failure mode: \"" + std::to_string((int)deletion_failure_mode + '"'));
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

void RenderingResources::add_charset(VariableAndHash<std::string> name, const std::u32string& charset) {
    UnorderedMap<char32_t, uint32_t> charset_map;
    for (const auto& [i, c] : enumerate(charset)) {
        charset_map.add(c, i);
    }
    charsets_.add(std::move(name), std::move(charset_map));
}

const std::unordered_map<char32_t, uint32_t>& RenderingResources::get_charset(const VariableAndHash<std::string>& name) const {
    return charsets_.get(name);
}

const LoadedFont& RenderingResources::get_font_texture(const FontNameAndHeight& font_descriptor) const
{
    {
        std::shared_lock lock{ font_mutex_ };
        if (auto it = font_textures_.try_get(font_descriptor); it != nullptr) {
            return *it;
        }
    }
    std::scoped_lock lock{ font_mutex_ };
    if (auto it = font_textures_.try_get(font_descriptor); it != nullptr) {
        return *it;
    }
    const auto& charset = charsets_.get(font_descriptor.charset);
    LoadedFont font{
        .cdata = std::vector<stbtt_bakedchar>(charset.size()),
        .texture_width = 1024,
        .texture_height = 1024
    };
    {
        std::vector<unsigned char> temp_bitmap(font.texture_width * font.texture_height);
        {
            std::vector<uint8_t> ttf_buffer = read_file_bytes(font_descriptor.ttf_filename);
            font.bottom_y = stbtt_BakeFontBitmap_get_y0(
                ttf_buffer.data(),
                0,  // font location (use offset=0 for plain .ttf)
                font_descriptor.height_pixels,
                temp_bitmap.data(),
                integral_cast<int>(font.texture_width),
                integral_cast<int>(font.texture_height),
                charset,
                font.cdata.data()); // no guarantee this fits!
        }
        CHK(glGenTextures(1, &font.texture_handle));
        CHK(glBindTexture(GL_TEXTURE_2D, font.texture_handle));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, integral_cast<GLsizei>(font.texture_width), integral_cast<GLsizei>(font.texture_height), 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data()));
        CHK(glBindTexture(GL_TEXTURE_2D, 0));
    }
    return add_font(font_textures_, font_descriptor, std::move(font));
}

void RenderingResources::save_to_file(
    const std::string& filename,
    const ColormapWithModifiers& color,
    TextureRole role) const
{
    if (!filename.ends_with(".png")) {
        THROW_OR_ABORT("Filename \"" + filename + "\" does not end with .png");
    }
    auto img = get_texture_data(color, role, FlipMode::NONE);
    if (!stbi_write_png(
        filename.c_str(),
        img->width,
        img->height,
        img->nrChannels,
        img->data(),
        0))
    {
        THROW_OR_ABORT("Could not write \"" + filename + '"');
    }
}

void RenderingResources::save_array_to_file(
    const std::string& filename_prefix,
    const ColormapWithModifiers& color,
    TextureRole role) const
{
    for (const auto& [i, img] : enumerate(get_texture_array_data(color, role, FlipMode::NONE)))
    {
        auto filename = filename_prefix + std::to_string(i) + ".png";
        if (!stbi_write_png(
            filename.c_str(),
            img->width,
            img->height,
            img->nrChannels,
            img->data(),
            0))
        {
            THROW_OR_ABORT("Could not write \"" + filename + '"');
        }
    }
}

void RenderingResources::add_texture(
    const ColormapWithModifiers& name,
    std::vector<std::byte>&& data,
    FlipMode flip_mode,
    TextureAlreadyExistsBehavior already_exists_behavior)
{
    const auto& filename = (const std::string&)name.filename;
    LOG_FUNCTION("RenderingResources::set_texture " + filename);
    std::scoped_lock lock{ mutex_ };

    if (preloaded_texture_dds_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "DDS-texture with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("DDS-texture with name \"" + filename + "\" already exists");
    }
    if (preloaded_processed_texture_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded processed non-DDS-texture with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded processed non-DDS-texture with name \"" + filename + "\" already exists");
    }
    if (preloaded_processed_texture_array_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded processed non-DDS-texture with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded processed non-DDS-texture with name \"" + filename + "\" already exists");
    }
    if (preloaded_raw_texture_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded raw non-DDS-texture with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded raw non-DDS-texture with name \"" + filename + "\" already exists");
    }
    if (auto_atlas_tile_descriptors_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Auto texture atlas with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Auto texture atlas with name \"" + filename + "\" already exists");
    }
    if (textures_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Non-DDS-texture with name \"" + filename + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Non-DDS-texture with name \"" + filename + "\" already exists");
    }
    auto extension = std::filesystem::path{ filename }.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
        ::tolower);
    if ((extension == ".jpg") ||
        (extension == ".png") ||
        (extension == ".bmp"))
    {
        add(preloaded_raw_texture_data_, name, std::move(data), flip_mode);
    } else if (extension == ".dds") {
        add(preloaded_texture_dds_data_, name, std::move(data), flip_mode);
    } else {
        THROW_OR_ABORT("Unknown file extension: \"" + filename + '"');
    }
}

std::pair<GLuint, TextureType> RenderingResources::initialize_non_dds_texture(const ColormapWithModifiers& color, TextureRole role, float aniso) const
{
    if (role == TextureRole::COLOR_FROM_DB) {
        return initialize_non_dds_texture(colormap(color), TextureRole::COLOR, aniso);
    }
    auto chk_type = [&](TextureType actual_texture_type){
        auto expected_texture_type = texture_type(color, role);
        if (expected_texture_type != actual_texture_type) {
            THROW_OR_ABORT(
                (std::stringstream() << "Unexpected texture return type (conflicting interpolation modes?). " <<
                "Expected: " << expected_texture_type <<
                ", Actual: " << actual_texture_type <<
                ", Color: " << color).str());
        }
        return actual_texture_type;
        };
    auto generate_texture = [&color, &aniso](
        const uint8_t* data,
        int width,
        int height,
        int nrChannels)
    {
        GLuint texture;
        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(GL_TEXTURE_2D, texture));
        if (aniso != 0) {
            CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
        CHK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));  // https://stackoverflow.com/a/49126350/2292832
        {
#ifdef __ANDROID__
            auto nchannels = (size_t)nrChannels;
#else
            auto nchannels = max(color.color_mode);
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
        CHK(glBindTexture(GL_TEXTURE_2D, 0));
        return texture;
    };
    auto generate_texture_array = [&color, &aniso, &chk_type](const std::vector<std::shared_ptr<StbInfo<uint8_t>>>& data) -> std::pair<GLuint, TextureType>
    {
        if (data.empty()) {
            THROW_OR_ABORT("Texture array is empty");
        }
        std::vector<uint8_t> flat_data;
        auto layer_size = integral_cast<size_t>(data[0]->width * data[0]->height * data[0]->nrChannels);
        flat_data.reserve(layer_size * data.size());
        for (const auto& d : data) {
            if ((data[0]->width != d->width) ||
                (data[0]->height != d->height) ||
                (data[0]->nrChannels != d->nrChannels)) {
                THROW_OR_ABORT("Texture array size mismatch");
            }
            flat_data.insert(flat_data.end(), d->data(), d->data() + layer_size);
        }
        GLuint texture;
        GLenum target = (color.depth_interpolation == InterpolationMode::NEAREST)
            ? GL_TEXTURE_2D_ARRAY
            : GL_TEXTURE_3D;
        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(target, texture));
        if (aniso != 0) {
            CHK(glTexParameterf(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
        CHK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));  // https://stackoverflow.com/a/49126350/2292832
        {
#ifdef __ANDROID__
            auto nchannels = (size_t)data[0]->nrChannels;
#else
            auto nchannels = max(color.color_mode);
#endif
            CHK(glTexImage3D(
                target,
                0,
                nchannels2internal_format(nchannels),
                data[0]->width,
                data[0]->height,
                integral_cast<GLsizei>(data.size()),
                0,
                nchannels2format((size_t)data[0]->nrChannels),
                GL_UNSIGNED_BYTE,
                flat_data.data()));
        }

        if (color.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
            CHK(glGenerateMipmap(target));
        }
        CHK(glBindTexture(target, 0));
        return { texture, chk_type(target == GL_TEXTURE_2D_ARRAY ? TextureType::TEXTURE_2D_ARRAY : TextureType::TEXTURE_3D) };
    };

    if (auto it = get_or_extract<EXTRACT_PROCESSED>(preloaded_processed_texture_data_, color); it != nullptr) {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Using preloaded texture: " << color;
        }
        return { generate_texture((*it)->data(), (*it)->width, (*it)->height, (*it)->nrChannels), TextureType::TEXTURE_2D };
    } else if (auto it = get_or_extract<EXTRACT_PROCESSED>(preloaded_processed_texture_array_data_, color); it != nullptr) {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Using preloaded texture array: " << color;
        }
        if (it->size() == 1) {
            const auto& data = *((*it)[0]);
            return { generate_texture(data.data(), data.width, data.height, data.nrChannels), chk_type(TextureType::TEXTURE_2D) };
        } else {
            return generate_texture_array(*it);
        }
    } else if (auto it = get_or_extract<EXTRACT_RAW>(preloaded_raw_texture_data_, color); it != nullptr) {
        auto si = stb_load8(*color.filename, FlipMode::NONE, &it->data, IncorrectDatasizeBehavior::CONVERT);
        return { generate_texture(si.data(), si.width, si.height, si.nrChannels), chk_type(TextureType::TEXTURE_2D) };
    } else {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Could not find preloaded texture: " << color;
        }
        if (auto it = manual_atlas_tile_descriptors_.try_get(color.filename); it != nullptr) {
            auto sis = get_texture_array_data(color, role, FlipMode::VERTICAL);
            if (it->nlayers == 1) {
                const auto& data = *sis[0];
                return { generate_texture(data.data(), data.width, data.height, data.nrChannels), chk_type(TextureType::TEXTURE_2D) };
            } else {
                return generate_texture_array(sis);
            }
        } else {
            auto si = get_texture_data(color, role, FlipMode::VERTICAL);
            return { generate_texture(si->data(), si->width, si->height, si->nrChannels), chk_type(TextureType::TEXTURE_2D) };
        }
    }
}

std::shared_ptr<ITextureHandle> RenderingResources::initialize_dds_texture(
    const ColormapWithModifiers& color,
    float aniso) const
{
    auto data = get_or_extract<EXTRACT_RAW>(preloaded_texture_dds_data_, color);

    nv_dds::CDDSImage image;
    {
        std::stringstream sstr;
        for (std::byte c : data->data) {
            sstr << (uint8_t)c;
        }
        // Setting flipImage to false because it does not
        // work with image or mipmap sizes that are not a
        // multiple of 4.
        // Instead, the textures are now flipped using the GPU.
        image.load(sstr, false /* flipImage */);
    }

    auto handle = std::make_shared<Texture>(
        generate_texture,
        color_mode_from_channels(image.get_components()),
        MipmapMode::WITH_MIPMAPS,
        color.wrap_modes);

    BindTextureGuard btg{ GL_TEXTURE_2D, handle->handle<GLuint>() };

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
    TextureSizeAndMipmaps original_texture{
        .handle = handle,
        .width = integral_cast<GLsizei>(image.get_width()),
        .height = integral_cast<GLsizei>(image.get_height()),
        .nchannels = integral_cast<GLsizei>(image.get_components()),
        .mip_level_count = integral_cast<GLsizei>(image.get_num_mipmaps()),
        .wrap_s = wrap_mode_to_native(color.wrap_modes(0)),
        .wrap_t = wrap_mode_to_native(color.wrap_modes(1))};
    if (data->flip_mode == FlipMode::NONE) {
        return original_texture.handle;
    } else if (data->flip_mode == FlipMode::VERTICAL) {
        return original_texture.flipped_vertically(aniso);
    } else {
        THROW_OR_ABORT("Unsupported flip mode");
    }
}
