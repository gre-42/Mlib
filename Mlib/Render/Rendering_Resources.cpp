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
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Clear_Wrapper.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Allocator.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Render/Deallocate/Render_Try_Delete.hpp>
#include <Mlib/Render/Instance_Handles/Array_Frame_Buffer.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Render_Texture_Atlas.hpp>
#include <Mlib/Render/Text/Loaded_Font.hpp>
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
#include <stb_cpp/stb_image_atlas.hpp>
#include <stb_cpp/stb_image_load.hpp>
#include <stb_cpp/stb_lighten.hpp>
#include <stb_cpp/stb_mipmaps.hpp>
#include <stb_cpp/stb_set_alpha.hpp>
#include <stb_cpp/stb_truetype_aligned.hpp>
#include <string>
#include <vector>

using namespace Mlib;
namespace fs = std::filesystem;

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

static StbInfo<uint8_t> stb_load_texture(const std::string& filename,
                                int nchannels,
                                FlipMode flip_mode) {
    auto result = stb_load8(filename, flip_mode, nullptr, IncorrectDatasizeBehavior::CONVERT);
    if (result.nrChannels < std::abs(nchannels)) {
        THROW_OR_ABORT(filename + " does not have at least " + std::to_string(nchannels) + " channels");
    }
    if (!is_power_of_two(result.width) || !is_power_of_two(result.height)) {
        lwarn() << filename << " size: " << result.width << 'x' << result.height;
    }
    if ((nchannels > 0) && (result.nrChannels != nchannels)) {
        lwarn() << filename << " #channels: " << result.nrChannels;
    }
    return result;
}

static StbInfo<uint8_t> stb_load_and_transform_texture(const TextureDescriptor& desc, FlipMode flip_mode) {
    std::string touch_file = desc.color.filename + ".xpltd";
    if ((desc.color_mode == ColorMode::RGBA) &&
        desc.color.alpha.empty() &&
        getenv_default_bool("EXTRAPOLATE_COLORS", false) &&
        !fs::exists(touch_file))
    {
        linfo() << "Extrapolating RGBA image \"" << desc.color << '"';
        auto img = StbImage4::load_from_file(desc.color.filename);
        float sigma = 3.f;
        size_t niterations = 1 + (size_t)((float)std::max(img.shape(0), img.shape(1)) / (sigma * 4));
        extrapolate_rgba_colors(
            img,
            sigma,
            niterations).save_to_file(desc.color.filename);
        std::ofstream ofstr{ touch_file };
        if (ofstr.fail()) {
            THROW_OR_ABORT("Could not create file \"" + touch_file + '"');
        }
    }
    StbInfo<uint8_t> si0;
    if (!desc.color.alpha.empty()) {
        if (desc.color_mode != ColorMode::RGBA) {
            THROW_OR_ABORT("Color mode not RGBA despite alpha texture: \"" + desc.color.filename + '"');
        }
        si0 = stb_load_texture(
            desc.color.filename, (int)ColorMode::RGB, flip_mode);
        if (si0.nrChannels != 3) {
            THROW_OR_ABORT("#channels not 3: \"" + desc.color.filename + '"');
        }
        auto si_alpha = stb_load_texture(
            desc.color.alpha, (int)ColorMode::GRAYSCALE, flip_mode);
        if (si_alpha.nrChannels != 1) {
            THROW_OR_ABORT("#channels not 1: \"" + desc.color.alpha + '"');
        }
        if ((si_alpha.width != si0.width) ||
            (si_alpha.height != si0.height))
        {
            THROW_OR_ABORT("Size mismatch between files \"" + desc.color.filename + "\" and \"" + desc.color.alpha + '"');
        }
        StbInfo<uint8_t> si0_rgb = std::move(si0);
        si0 = stb_create<uint8_t>(si0_rgb.width, si0_rgb.height, 4);
        stb_set_alpha(
            si0_rgb.data.get(),
            si_alpha.data.get(),
            si0.data.get(),
            si0.width,
            si0.height);
    } else {
        si0 = stb_load_texture(
            desc.color.filename, (int)desc.color_mode, flip_mode);
    }
    if (!desc.color.average.empty()) {
        auto si1 = stb_load_texture(
            desc.color.average, (int)desc.color_mode, flip_mode);
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
    if (!desc.color.multiply.empty()) {
        auto si1 = stb_load_texture(
            desc.color.multiply, (int)desc.color_mode, flip_mode);
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
    if (desc.alpha_fac != 1.f) {
        stb_alpha_fac(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            desc.alpha_fac);
    }
    if (desc.color.desaturate) {
        stb_desaturate(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels);
    }
    if (!desc.color.histogram.empty()) {
        Array<unsigned char> image = stb_image_2_array(si0);
        Array<unsigned char> ref = stb_image_2_array(stb_load_texture(desc.color.histogram, -3, FlipMode::NONE));
        Array<unsigned char> m = match_rgba_histograms(image, ref);
        assert_true(m.shape(0) == (size_t)si0.nrChannels);
        assert_true(m.shape(1) == (size_t)si0.height);
        assert_true(m.shape(2) == (size_t)si0.width);
        array_2_stb_image(m, si0.data.get());
    }
    if (!desc.color.lighten.all_equal(0.f)) {
        const FixedArray<float, 3>& lighten = desc.color.lighten;
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
            (desc.color.lighten * 255.f).casted<short>().flat_begin());
    }
    if (!desc.color.lighten_left.all_equal(0.f) ||
        !desc.color.lighten_right.all_equal(0.f))
    {
        const FixedArray<float, 3>& lighten_left = desc.color.lighten_left;
        const FixedArray<float, 3>& lighten_right = desc.color.lighten_right;
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
            (desc.color.lighten_left * 255.f).casted<short>().flat_begin(),
            (desc.color.lighten_right * 255.f).casted<short>().flat_begin());
    }
    if (!desc.color.lighten_top.all_equal(0.f) ||
        !desc.color.lighten_bottom.all_equal(0.f))
    {
        const FixedArray<float, 3>& lighten_top = desc.color.lighten_top;
        const FixedArray<float, 3>& lighten_bottom = desc.color.lighten_bottom;
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
            (desc.color.lighten_top * 255.f).casted<short>().flat_begin(),
            (desc.color.lighten_bottom * 255.f).casted<short>().flat_begin());
    }
    if (!desc.color.mean_color.all_equal(-1.f)) {
        if (!stb_colorize(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.color.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            lwarn() << "alpha = 0: " << desc.color << std::endl;
        }
    }
    if (!desc.color.alpha_blend.empty()) {
        auto si1 = stb_load_texture(
            desc.color.alpha_blend, 4, flip_mode);
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
    : preloaded_texture_data_{"Preloaded texture dataa",
                              [](const ColormapWithModifiers &e) { return e.filename; }}
    , preloaded_texture_dds_data_{"Preloaded texture DDS data"}
    , texture_descriptors_{"Texture descriptors"}
    , textures_{"Textures", [](const ColormapWithModifiers &e) { return e.filename; }}
    , manual_atlas_tile_descriptors_{"Manual atlas tile descriptors"}
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
    , deallocation_token_{render_deallocator.insert([this]() { deallocate(); })} {
}

RenderingResources::~RenderingResources() {
    deallocate();
}

void RenderingResources::deallocate() {
    std::scoped_lock lock{mutex_};
    render_programs_.clear();
    textures_.erase_if([](auto &item) {
        auto &[_, texture] = item;
        if (texture.needs_gc) {
            try_delete_texture(const_cast<GLuint &>(texture.handle));
            return true;
        }
        return false;
    });
    font_textures_.clear();
}

void RenderingResources::preload(const TextureDescriptor& descriptor) const {
    LOG_FUNCTION("RenderingResources::preload, color=" + descriptor.color);
    std::scoped_lock lock{ mutex_ };
    auto dit = texture_descriptors_.try_get(descriptor.color.filename);
    const TextureDescriptor& desc = dit != nullptr
        ? *dit
        : descriptor;
    if (ContextQuery::is_initialized()) {
        if (!desc.color.filename.empty()) {
            get_texture(desc, CallerType::PRELOAD);
        }
        if (!desc.normal.filename.empty()) {
            get_normalmap_texture(desc);
        }
    } else {
        if (!desc.color.filename.empty() &&
            !textures_.contains(descriptor.color) &&
            !preloaded_texture_data_.contains(descriptor.color) &&
            !preloaded_texture_dds_data_.contains(descriptor.color.filename) &&
            !auto_atlas_tile_descriptors_.contains(descriptor.color.filename))
        {
            auto data = get_texture_data(desc, FlipMode::VERTICAL);
            preloaded_texture_data_.emplace(descriptor.color, std::move(data));
            if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                linfo() << this << " Preloaded color texture: " << descriptor.color;
            }
        }
        if (!desc.normal.filename.empty() &&
            !textures_.contains({.filename = desc.normal.filename, .average = desc.normal.average}) &&
            !preloaded_texture_data_.contains({.filename = desc.normal.filename, .average = desc.normal.average}) &&
            !preloaded_texture_dds_data_.contains(desc.normal.filename) &&
            !auto_atlas_tile_descriptors_.contains(descriptor.normal.filename))
        {
            auto data = get_texture_data(
                TextureDescriptor{
                    .color = {.filename = desc.normal.filename, .average = desc.normal.average},
                    .color_mode = ColorMode::RGB},
                FlipMode::VERTICAL);
            preloaded_texture_data_.emplace({.filename = desc.normal.filename, .average = desc.normal.average}, std::move(data));
            if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                linfo() << this << " Preloaded normal texture: " << desc.normal;
            }
        }
    }
}

bool RenderingResources::texture_is_loaded_and_try_preload(const TextureDescriptor& descriptor) {
    if (texture_is_loaded_unsafe(descriptor.color)) {
        return true;
    }
    if (!preloader_background_loop_.done()) {
        return false;
    }
    std::scoped_lock lock{mutex_};
    if (texture_is_loaded_unsafe(descriptor.color)) {
        return true;
    }
    if (preloader_background_loop_.done()) {
        preloader_background_loop_.run([this, descriptor](){
            preload(descriptor);
        });
    }
    return false;
}

bool RenderingResources::texture_is_loaded_unsafe(const ColormapWithModifiers& name) const {
    if (preloaded_texture_dds_data_.contains(name.filename)) {
        return true;
    }
    if (preloaded_texture_data_.contains(name)) {
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

std::string RenderingResources::get_texture_filename(
    const TextureDescriptor& descriptor,
    const std::string& default_filename) const
{
    std::shared_lock lock{mutex_};
    auto dit = texture_descriptors_.try_get(descriptor.color.filename);
    const TextureDescriptor &desc = dit != nullptr
        ? *dit
        : descriptor;
    
    if (manual_atlas_tile_descriptors_.contains(desc.color.filename) ||
        desc.color.desaturate ||
        !desc.color.histogram.empty() ||
        !desc.color.average.empty() ||
        !desc.color.multiply.empty() ||
        !desc.color.mean_color.all_equal(-1.f) ||
        !desc.color.lighten.all_equal(0.f))
    {
        StbInfo si = get_texture_data(desc, FlipMode::VERTICAL);
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
        return desc.color.filename;
    }
}

GLuint RenderingResources::get_texture(
    const TextureDescriptor& descriptor,
    CallerType caller_type) const
{
    return get_texture(descriptor.color, descriptor, caller_type);
}

GLuint RenderingResources::get_normalmap_texture(const TextureDescriptor& descriptor) const {
    return get_texture(TextureDescriptor{
        .color = {
            .filename = descriptor.normal.filename,
            .average = descriptor.normal.average},
        .color_mode = ColorMode::RGB,
        .mipmap_mode = descriptor.mipmap_mode,
        .anisotropic_filtering_level = descriptor.anisotropic_filtering_level});
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

// From: https://gamedev.stackexchange.com/questions/70829/why-is-gl-texture-max-anisotropy-ext-undefined/75816#75816?newreg=a7ddca6a76bf40b794c36dbe189c64b6
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

GLuint RenderingResources::get_texture(
    const ColormapWithModifiers& name,
    const TextureDescriptor& descriptor,
    CallerType caller_type) const
{
    LOG_FUNCTION("RenderingResources::get_texture " + name.filename);
    if (auto it = textures_.try_get(name); it != nullptr) {
        return it->handle;
    }
    std::scoped_lock lock{mutex_};
    if (auto it = textures_.try_get(name); it != nullptr) {
        return it->handle;
    }
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    RecursionGuard rg{recursion_counter};
    auto dit = texture_descriptors_.try_get(name.filename);
    const TextureDescriptor &desc = dit != nullptr
        ? *dit
        : descriptor;
    if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
        linfo() << this << " Loading texture: " << desc.color;
        if (!desc.color.average.empty()) {
            linfo() << this << " Loading texture: " << desc.color.average;
        }
        if (!desc.color.multiply.empty()) {
            linfo() << this << " Loading texture: " << desc.color.multiply;
        }
    }

    float aniso = 0.0f;
    WARN(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
    aniso = std::min({ aniso, (float)desc.anisotropic_filtering_level, (float)max_anisotropic_filtering_level_ });

    GLuint texture;

    if (auto ait = auto_atlas_tile_descriptors_.try_extract(descriptor.color.filename); !ait.empty()) {
        if (caller_type != CallerType::PRELOAD) {
            THROW_OR_ABORT("Texture source is not preload for texture \"" + descriptor.color.filename + '"');
        }
        const auto &adesc = ait.mapped();
        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, texture));
        CHK(glTexStorage3D(
            GL_TEXTURE_2D_ARRAY,
            adesc.mip_level_count,
            nchannels2sized_internal_format((GLenum)adesc.color_mode),
            adesc.width,
            adesc.height,
            integral_cast<GLsizei>(adesc.tiles.size())));
        if ((desc.anisotropic_filtering_level != 0) &&
            (max_anisotropic_filtering_level_ != 0) &&
            (aniso != 0))
        {
            CHK(glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
        CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, 0));
        for (const auto& [layer, tiles] : enumerate(adesc.tiles)) {
            for (int level = 0; level < adesc.mip_level_count; ++level) {
                ArrayFrameBufferStorage afbs{texture, level, integral_cast<int>(layer)};
                clear_color({0.f, 0.f, 0.f, 0.f});
                CHK(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
                render_texture_atlas(tiles, *this, level);
                // // Disable the ArrayFrameBufferStorage above for the following code to work.
                // static SaveMovie save_movie;
                // save_movie.save(
                //     "/tmp/atlas_",
                //     "_layer",
                //     integral_cast<size_t>(adesc.width / (1 << level)),
                //     integral_cast<size_t>(adesc.height / (1 << level)));
            }
        }
    } else {
        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(GL_TEXTURE_2D, texture));
        if ((desc.anisotropic_filtering_level != 0) &&
            (max_anisotropic_filtering_level_ != 0) &&
            (aniso != 0))
        {
            CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
        if (preloaded_texture_dds_data_.contains(name.filename)) {
            initialize_dds_texture(name.filename, desc);
        } else {
            initialize_non_dds_texture(desc.color, desc);
        }
        CHK(glBindTexture(GL_TEXTURE_2D, 0));
    }

    textures_.emplace(name, TextureHandleAndNeedsGc{texture, true});
    return texture;
}

GLuint RenderingResources::get_cubemap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_cubemap " + name);
    if (auto it = textures_.try_get({.filename = name}); it != nullptr) {
        return it->handle;
    }
    std::scoped_lock lock{mutex_};
    if (auto it = textures_.try_get({.filename = name}); it != nullptr) {
        return it->handle;
    }
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
        const TextureDescriptor& desc = dit != nullptr
            ? *dit
            : TextureDescriptor{
                .color = {.filename = color},
                .color_mode = ColorMode::RGB};

        auto info = get_texture_data(desc, FlipMode::NONE);
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

    textures_.emplace({.filename = name}, TextureHandleAndNeedsGc{textureID, true});
    return textureID;
}

void RenderingResources::set_texture(const std::string& name, GLuint id) {
    LOG_FUNCTION("RenderingResources::set_texture " + name);
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        THROW_OR_ABORT("RenderingResources::set_texture: invalid texture ID");
    }
    auto it = textures_.try_extract({.filename = name});
    if (!it.empty() && it.mapped().needs_gc) {
        THROW_OR_ABORT("Overwriting texture that needs garbabe-collection");
    }
    textures_.emplace({.filename = name}, TextureHandleAndNeedsGc{id, false});
}

void RenderingResources::add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + name);
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
        {preload({.color = {.filename = name}, .color_mode = texture_atlas_descriptor.color_mode});});
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
    const TextureDescriptor& descriptor,
    FlipMode flip_mode) const
{
    if (auto it = manual_atlas_tile_descriptors_.try_get(descriptor.color.filename); it != nullptr) {
        auto si = stb_create<uint8_t>(it->width, it->height, (int)it->color_mode);
        std::vector<AtlasTile> atlas_tiles;
        atlas_tiles.reserve(it->tiles.size());
        for (const auto& atd : it->tiles) {
            auto dit = texture_descriptors_.try_get(atd.filename);
            TextureDescriptor desc = dit != nullptr
                ? *dit
                : TextureDescriptor{
                    .color = {.filename = atd.filename},
                    .color_mode = descriptor.color_mode};
            atlas_tiles.push_back(AtlasTile{
                .left = atd.left,
                .bottom = atd.bottom,
                .image = get_texture_data(desc, flip_mode)});
        }
        build_image_atlas(si, atlas_tiles);
        return si;
    }
    auto si = stb_load_and_transform_texture(descriptor, flip_mode);
    if ((descriptor.color_mode == ColorMode::RGB) &&
        (si.nrChannels == 4) &&
        getenv_default_bool("CHECK_OPACITY", false))
    {
        double opacity = mean_opacity(si);
        if (opacity < 0.8) {
            lwarn() << descriptor.color << ": Opacity is only " << opacity;
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
    AutoTextureAtlasDescriptor* atlas)
{
    std::map<std::string, FixedArray<int, 2>> packed_sizes;
    for (const auto& filename : filenames) {
        FixedArray<int, 2> image_size;
        if (preloaded_texture_data_.contains({.filename = filename})) {
            const auto& img = preloaded_texture_data_.get({.filename = filename});
            image_size = {img.width, img.height};
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
    FixedArray<int, 2> atlas_size_2d{4096, 4096};
    auto packed_boxes = pack_boxes(packed_sizes, atlas_size_2d);
    AutoTextureAtlasDescriptor tad{
        .width = atlas_size_2d(0),
        .height = atlas_size_2d(1),
        .mip_level_count = 3,
        .color_mode = ColorMode::RGBA,
        .tiles = {}
    };
    if ((size_t)tad.width * (size_t)tad.height * packed_boxes.size() > 4096 * 4096 * 20) {
        THROW_OR_ABORT("Atlas too large");
    }
    std::map<std::string, AutoUvTile> result;
    for (const auto& [layer, nbs] : enumerate(packed_boxes)) {
        auto& tiles = tad.tiles.emplace_back();
        for (const auto& nb : nbs) {
            auto size_it = packed_sizes.find(nb.name);
            if (size_it == packed_sizes.end()) {
                THROW_OR_ABORT("Could not find texture with name \"" + nb.name + '"');
            }
            const auto& size = size_it->second;
            if (!result.insert({
                nb.name,
                AutoUvTile{
                    .position = nb.bottom_left.casted<float>() / (atlas_size_2d.casted<float>() - 1.f),
                    .size = size.casted<float>()
                            / FixedArray<float, 2>{(float)tad.width, (float)tad.height},
                    .layer = integral_cast<uint8_t>(layer)}}).second)
            {
                THROW_OR_ABORT("Detected duplicate atlas filename: \"" + nb.name + '"');
            }
            tiles.push_back(AutoAtlasTileDescriptor{
                .left = nb.bottom_left(0),
                .bottom = nb.bottom_left(1),
                .width = size(0),
                .height = size(1),
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
    return BlendMapTexture{.texture_descriptor = {.color = {.filename = name},
                                                  .mipmap_mode = MipmapMode::WITH_MIPMAPS}};
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
void RenderingResources::delete_texture(const std::string& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_texture " + name);
    if (textures_.erase({.filename = name}) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            lwarn() << "Could not delete texture " << name;
        } else {
            THROW_OR_ABORT("Could not delete texture " + name);
        }
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

void RenderingResources::save_to_file(const std::string& filename, const TextureDescriptor& desc) const {
    if (!filename.ends_with(".png")) {
        THROW_OR_ABORT("Filename \"" + filename + "\" does not end with .png");
    }
    StbInfo img = get_texture_data(desc, FlipMode::NONE);
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
    if (preloaded_texture_data_.contains({.filename = name})) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::WARN) {
            lwarn() << "Preloaded non-DDS-texture with name \"" + name + "\" already exists";
            return;
        }
        THROW_OR_ABORT("Preloaded non-DDS-texture with name \"" + name + "\" already exists");
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
        (extension == ".png"))
    {
        auto d = std::move(data);
        auto image = stb_load8(name, FlipMode::NONE, &d, IncorrectDatasizeBehavior::CONVERT);
        preloaded_texture_data_.emplace({.filename = name}, std::move(image));
    } else if (extension == ".dds") {
        preloaded_texture_dds_data_.emplace(name, std::move(data));
    } else {
        THROW_OR_ABORT("Unknown file extension: \"" + name + '"');
    }
}

void RenderingResources::initialize_non_dds_texture(
    const ColormapWithModifiers& name,
    const TextureDescriptor& descriptor) const
{
    StbInfo<uint8_t> si;
    if (auto it = preloaded_texture_data_.try_extract(name); !it.empty()) {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Using preloaded texture: " << name;
        }
        si = std::move(it.mapped());
    } else {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Could not find preloaded texture: " << name;
        }
        si = get_texture_data(descriptor, FlipMode::VERTICAL);
    }
    CHK(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));  // https://stackoverflow.com/a/49126350/2292832
    CHK(glTexImage2D(GL_TEXTURE_2D,
                     0,
                     nchannels2internal_format((GLenum)descriptor.color_mode),
                     si.width,
                     si.height,
                     0,
                     nchannels2format((size_t)si.nrChannels),
                     GL_UNSIGNED_BYTE,
                     si.data.get()));
    // if (si.nrChannels == 4) {
    //     generate_rgba_mipmaps_inplace(si);
    // } else {
    //     CHK(glGenerateMipmap(GL_TEXTURE_2D));
    // }
    if (descriptor.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
        CHK(glGenerateMipmap(GL_TEXTURE_2D));
    }
}

void RenderingResources::initialize_dds_texture(const std::string& name, const TextureDescriptor& descriptor) const
{
    auto it = preloaded_texture_dds_data_.extract(name);

    nv_dds::CDDSImage image;
    {
        std::stringstream sstr;
        for (uint8_t c : it.mapped()) {
            sstr << c;
        }
        image.load(sstr);
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
                GL_UNSIGNED_BYTE,
                image));
        }
        if (descriptor.mipmap_mode == MipmapMode::WITH_MIPMAPS) {
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
                GL_UNSIGNED_BYTE,
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
                    GL_UNSIGNED_BYTE,
                    mipmap));
            }
        }
    }
}
