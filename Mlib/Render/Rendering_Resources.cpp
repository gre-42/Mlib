#include "Rendering_Resources.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Geometry/Mesh/Uv_Tile.hpp>
#include <Mlib/Geometry/Pack_Boxes.hpp>
#include <Mlib/Images/Dds_Info.hpp>
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
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
    std::string touch_file = desc.color + ".xpltd";
    if ((desc.color_mode == ColorMode::RGBA) &&
        desc.alpha.empty() &&
        getenv_default_bool("EXTRAPOLATE_COLORS", false) &&
        !fs::exists(touch_file))
    {
        linfo() << "Extrapolating RGBA image \"" << desc.color << '"';
        auto img = StbImage4::load_from_file(desc.color);
        float sigma = 3.f;
        size_t niterations = 1 + (size_t)((float)std::max(img.shape(0), img.shape(1)) / (sigma * 4));
        extrapolate_rgba_colors(
            img,
            sigma,
            niterations).save_to_file(desc.color);
        std::ofstream ofstr{ touch_file };
        if (ofstr.fail()) {
            THROW_OR_ABORT("Could not create file \"" + touch_file + '"');
        }
    }
    StbInfo<uint8_t> si0;
    if (!desc.alpha.empty()) {
        if (desc.color_mode != ColorMode::RGBA) {
            THROW_OR_ABORT("Color mode not RGBA despite alpha texture: \"" + desc.color + '"');
        }
        si0 = stb_load_texture(
            desc.color, (int)ColorMode::RGB, flip_mode);
        if (si0.nrChannels != 3) {
            THROW_OR_ABORT("#channels not 3: \"" + desc.color + '"');
        }
        auto si_alpha = stb_load_texture(
            desc.alpha, (int)ColorMode::GRAYSCALE, flip_mode);
        if (si_alpha.nrChannels != 1) {
            THROW_OR_ABORT("#channels not 1: \"" + desc.alpha + '"');
        }
        if ((si_alpha.width != si0.width) ||
            (si_alpha.height != si0.height))
        {
            THROW_OR_ABORT("Size mismatch between files \"" + desc.color + "\" and \"" + desc.alpha + '"');
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
            desc.color, (int)desc.color_mode, flip_mode);
    }
    if (!desc.mixed.empty()) {
        auto si1_raw = stb_load_texture(
            desc.mixed, (int)desc.color_mode, flip_mode);
        std::unique_ptr<unsigned char[]> si1_resized{
            new unsigned char[(size_t)(si0.width * si0.height * si1_raw.nrChannels)]};
        stbir_resize_uint8(si1_raw.data.get(),
                           si1_raw.width,
                           si1_raw.height,
                           0,
                           si1_resized.get(),
                           si0.width,
                           si0.height,
                           0,
                           si1_raw.nrChannels);
        //int max_dist = si0.width * overlap_npixels;
        int max_dist = 5;
        for (int r = 0; r < si0.height; ++r) {
            for (int c = 0; c < si0.width; ++c) {
                int dist = std::min(c, si0.width - c - 1);
                float fac;
                if (dist < max_dist) {
                    fac = float(dist) / (float)max_dist;
                } else {
                    fac = 1;
                }
                for (int d = 0; d < si0.nrChannels; ++d) {
                    int i0 = (r * si0.width + c) * si0.nrChannels + d;
                    int i1 = (r * si0.width + c) * si1_raw.nrChannels + d;
                    si0.data.get()[i0] =
                        (unsigned char)(fac * (float)si0.data.get()[i0] + (1 - fac) * (float)si1_resized.get()[i1]);
                }
            }
        }
    }
    if (desc.alpha_fac != 1.f) {
        stb_alpha_fac(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            desc.alpha_fac);
    }
    if (desc.desaturate) {
        stb_desaturate(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels);
    }
    if (!desc.histogram.empty()) {
        Array<unsigned char> image = stb_image_2_array(si0);
        Array<unsigned char> ref = stb_image_2_array(stb_load_texture(desc.histogram, -3, FlipMode::NONE));
        Array<unsigned char> m = match_rgba_histograms(image, ref);
        assert_true(m.shape(0) == (size_t)si0.nrChannels);
        assert_true(m.shape(1) == (size_t)si0.height);
        assert_true(m.shape(2) == (size_t)si0.width);
        array_2_stb_image(m, si0.data.get());
    }
    if (!desc.lighten.all_equal(0.f)) {
        const FixedArray<float, 3>& lighten = desc.lighten;
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
            (desc.lighten * 255.f).casted<short>().flat_begin());
    }
    if (!desc.lighten_top.all_equal(0.f) ||
        !desc.lighten_bottom.all_equal(0.f))
    {
        const FixedArray<float, 3>& lighten_top = desc.lighten_top;
        const FixedArray<float, 3>& lighten_bottom = desc.lighten_bottom;
        if (any(lighten_top > 1.f) ||
            any(lighten_top < -1.f) ||
            !all(Mlib::isfinite(lighten_top)))
        {
            THROW_OR_ABORT("Lighten top value out of bounds");
        }
        if (any(lighten_bottom > 1.f) ||
            any(lighten_bottom < -1.f) ||
            !all(Mlib::isfinite(lighten_bottom)))
        {
            THROW_OR_ABORT("Lighten bottom value out of bounds");
        }
        stb_lighten_vertical_gradient(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.lighten_top * 255.f).casted<short>().flat_begin(),
            (desc.lighten_bottom * 255.f).casted<short>().flat_begin());
    }
    if (!desc.mean_color.all_equal(-1.f)) {
        if (!stb_colorize(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            lwarn() << "alpha = 0: " << desc.color << std::endl;
        }
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

RenderingResources::RenderingResources(
    std::string name,
    unsigned int max_anisotropic_filtering_level)
: name_{ std::move(name) },
  max_anisotropic_filtering_level_{ max_anisotropic_filtering_level },
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

RenderingResources::~RenderingResources() {
    deallocate();
}

void RenderingResources::deallocate() {
    std::scoped_lock lock{mutex_};
    render_programs_.clear();
    std::erase_if(textures_, [](auto& item){
        auto& [_, texture] = item;
        if (texture.needs_gc) {
            try_delete_texture(const_cast<GLuint&>(texture.handle));
            return true;
        }
        return false;
    });
    font_textures_.clear();
}

void RenderingResources::preload(const TextureDescriptor& descriptor) const {
    LOG_FUNCTION("RenderingResources::preload, color=" + descriptor.color);
    std::scoped_lock lock{ mutex_ };
    auto dit = texture_descriptors_.find(descriptor.color);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    if (ContextQuery::is_initialized()) {
        if (!desc.color.empty()) {
            get_texture(desc, CallerType::PRELOAD);
        }
        if (!desc.normal.empty()) {
            get_normalmap_texture(desc);
        }
    } else {
        if (!desc.color.empty() &&
            !textures_.contains(descriptor.color) &&
            !preloaded_texture_data_.contains(descriptor.color) &&
            !preloaded_texture_dds_data_.contains(descriptor.color) &&
            !auto_atlas_tile_descriptors_.contains(descriptor.color))
        {
            if (!preloaded_texture_data_.insert({descriptor.color, get_texture_data(desc, FlipMode::VERTICAL)}).second) {
                THROW_OR_ABORT("Could not preload color");
            } else if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                linfo() << this << " Preloaded color texture: " << descriptor.color;
            }
        }
        if (!desc.normal.empty() &&
            !textures_.contains(desc.normal) &&
            !preloaded_texture_data_.contains(desc.normal) &&
            !preloaded_texture_dds_data_.contains(desc.normal) &&
            !auto_atlas_tile_descriptors_.contains(descriptor.normal))
        {
            if (!preloaded_texture_data_.insert({
                desc.normal,
                get_texture_data(
                    TextureDescriptor{
                        .color = desc.normal,
                        .color_mode = ColorMode::RGB},
                    FlipMode::VERTICAL)}).second)
            {
                THROW_OR_ABORT("Could not preload normal");
            } if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
                linfo() << this << " Preloaded normal texture: " << desc.normal;
            }
        }
    }
}

bool RenderingResources::contains_texture(const std::string& name) const {
    std::shared_lock lock{mutex_};
    return textures_.contains(name) || auto_atlas_tile_descriptors_.contains(name);
}

std::string RenderingResources::get_texture_filename(
    const TextureDescriptor& descriptor,
    const std::string& default_filename) const
{
    std::shared_lock lock{mutex_};
    auto dit = texture_descriptors_.find(descriptor.color);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    
    if (manual_atlas_tile_descriptors_.contains(desc.color) ||
        desc.desaturate ||
        !desc.histogram.empty() ||
        !desc.mixed.empty() ||
        !desc.mean_color.all_equal(-1.f) ||
        !desc.lighten.all_equal(0.f))
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
        return desc.color;
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
        .color = descriptor.normal,
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
    const std::string& name,
    const TextureDescriptor& descriptor,
    CallerType caller_type) const {
    LOG_FUNCTION("RenderingResources::get_texture " + name);
    std::scoped_lock lock{mutex_};
    if (auto it = textures_.find(name); it != textures_.end())
    {
        return it->second.handle;
    }
    static THREAD_LOCAL(RecursionCounter) recursion_counter = RecursionCounter{};
    RecursionGuard rg{recursion_counter};
    auto dit = texture_descriptors_.find(name);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
        linfo() << this << " Loading texture: " << desc.color;
        if (!desc.mixed.empty()) {
            linfo() << this << " Loading texture: " << desc.mixed;
        }
    }
    
    float aniso = 0.0f;
    CHK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
    aniso = std::min({ aniso, (float)desc.anisotropic_filtering_level, float(max_anisotropic_filtering_level_) });

    GLuint texture;

    if (auto_atlas_tile_descriptors_.contains(descriptor.color)) {
        if (caller_type != CallerType::PRELOAD) {
            THROW_OR_ABORT("Texture source is not preload for texture \"" + descriptor.color + '"');
        }
        const auto& adesc = auto_atlas_tile_descriptors_.at(descriptor.color);
        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, texture));
        CHK(glTexStorage3D(GL_TEXTURE_2D_ARRAY, adesc.mip_level_count, nchannels2sized_internal_format((GLenum)adesc.color_mode), adesc.width, adesc.height, integral_cast<GLsizei>(adesc.tiles.size())));
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
        if (preloaded_texture_dds_data_.contains(name)) {
            initialize_dds_texture(name, desc);
        } else {
            initialize_non_dds_texture(name, desc);
        }
        CHK(glBindTexture(GL_TEXTURE_2D, 0));
    }

    textures_.insert({name, TextureHandleAndNeedsGc{texture, true}});
    return texture;
}

GLuint RenderingResources::get_cubemap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_cubemap " + name);
    std::scoped_lock lock{mutex_};
    if (auto it = textures_.find(name); it != textures_.end()) {
        return it->second.handle;
    }
    auto it = cubemap_descriptors_.find(name);
    if (it == cubemap_descriptors_.end()) {
        THROW_OR_ABORT("Could not find cubemap \"" + name + '"');
    }
    if (it->second.filenames.size() != 6) {
        THROW_OR_ABORT("Cubemap does not have 6 filenames");
    }
    GLuint textureID;
    CHK(glGenTextures(1, &textureID));
    CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    for (GLuint i = 0; i < it->second.filenames.size(); i++) {
        StbInfo info =
            stb_load_texture(it->second.filenames[i],
                             3,       // nchannels
                             FlipMode::NONE);
        if (it->second.desaturate) {
            stb_desaturate(
                info.data.get(),
                info.width,
                info.height,
                info.nrChannels);
        }
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

    if (auto it2 = textures_.insert({name, TextureHandleAndNeedsGc{textureID, true}}); !it2.second) {
        THROW_OR_ABORT("Cubemap with name \"" + name + "\" already exists");
    }
    return textureID;
}

void RenderingResources::set_texture(const std::string& name, GLuint id) {
    LOG_FUNCTION("RenderingResources::set_texture " + name);
    std::scoped_lock lock{mutex_};
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        THROW_OR_ABORT("RenderingResources::set_texture: invalid texture ID");
    }
    textures_[name] = TextureHandleAndNeedsGc{id, false};
}

void RenderingResources::add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + name);
    std::scoped_lock lock{mutex_};
    if (auto it = texture_descriptors_.insert({name, descriptor}); !it.second) {
        THROW_OR_ABORT("Texture descriptor with name \"" + name + "\" already exists");
    }
}

TextureDescriptor RenderingResources::get_existing_texture_descriptor(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_existing_texture_descriptor " + name);
    std::shared_lock lock{mutex_};
    auto it = texture_descriptors_.find(name);
    if (it == texture_descriptors_.end()) {
        THROW_OR_ABORT("Could not find texture descriptor: \"" + name + '"');
    }
    return it->second;
}

void RenderingResources::add_manual_texture_atlas(
    const std::string& name,
    const ManualTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_manual_texture_atlas " + name);
    std::scoped_lock lock{mutex_};
    if (auto it = manual_atlas_tile_descriptors_.insert({name, texture_atlas_descriptor}); !it.second) {
        THROW_OR_ABORT("Manual atlas descriptor with name \"" + name + "\" already exists");
    } 
}

void RenderingResources::add_auto_texture_atlas(
    const std::string& name,
    const AutoTextureAtlasDescriptor& texture_atlas_descriptor)
{
    LOG_FUNCTION("RenderingResources::add_auto_texture_atlas " + name);
    std::scoped_lock lock{mutex_};
    append_render_allocator(
        [this, name, texture_atlas_descriptor]()
        {preload({.color = name, .color_mode = texture_atlas_descriptor.color_mode});});
    if (auto it = auto_atlas_tile_descriptors_.insert({name, texture_atlas_descriptor}); !it.second) {
        THROW_OR_ABORT("Auto atlas descriptor with name \"" + name + "\" already exists");
    } 
}

void RenderingResources::add_cubemap(const std::string& name, const std::vector<std::string>& filenames, bool desaturate) {
    LOG_FUNCTION("RenderingResources::add_cubemap " + name);
    std::scoped_lock lock{mutex_};
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        THROW_OR_ABORT("Texture with name \"" + name + "\" already exists");
    }
    if (!cubemap_descriptors_.insert({
        name,
        CubemapDescriptor{
            .filenames = filenames,
            .desaturate = desaturate}}).second)
    {
        THROW_OR_ABORT("Cubemap with name \"" + name + "\" already exists");
    }
}

StbInfo<uint8_t> RenderingResources::get_texture_data(
    const TextureDescriptor& descriptor,
    FlipMode flip_mode) const
{
    if (auto it = manual_atlas_tile_descriptors_.find(descriptor.color); it != manual_atlas_tile_descriptors_.end()) {
        auto si = stb_create<uint8_t>(it->second.width, it->second.height, (int)it->second.color_mode);
        std::vector<AtlasTile> atlas_tiles;
        atlas_tiles.reserve(it->second.tiles.size());
        for (const auto& atd : it->second.tiles) {
            auto dit = texture_descriptors_.find(atd.filename);
            TextureDescriptor desc = dit != texture_descriptors_.end()
                ? dit->second
                : TextureDescriptor{
                    .color = atd.filename,
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
        auto extension = std::filesystem::path{filename}.extension().string();
        std::transform(extension.begin(), extension.end(), extension.begin(),
            [](unsigned char c){ return std::tolower(c); });
        if ((extension == ".jpg") ||
            (extension == ".png"))
        {
            int comp;
            if (stbi_info(filename.c_str(), &image_size(0), &image_size(1), &comp) == 0) {
                THROW_OR_ABORT("Could not read size information from file \"" + filename + '"');
            }
        } else if (extension == ".dds") {
            const auto& d = preloaded_texture_dds_data_.find(filename);
            if (d == preloaded_texture_dds_data_.end()) {
                THROW_OR_ABORT("Could not find DDS texture with name \"" + filename + '"');
            }
            auto info = DdsInfo::load_from_buffer(d->second);
            image_size = {info.width, info.height};
        } else {
            THROW_OR_ABORT("Unknown file extension: \"" + filename + '"');
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

BlendMapTexture RenderingResources::get_blend_map_texture(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_blend_map_texture " + name);
    std::shared_lock lock{mutex_};
    if (auto bit = blend_map_textures_.find(name); bit == blend_map_textures_.end()) {
        if (auto tit = texture_descriptors_.find(name); tit != texture_descriptors_.end()) {
            return BlendMapTexture{ .texture_descriptor = {
                .color = name,
                .alpha = tit->second.alpha,
                .specular = tit->second.specular,
                .normal = tit->second.normal,
                .mipmap_mode = tit->second.mipmap_mode,
                .anisotropic_filtering_level = tit->second.anisotropic_filtering_level } };
        } else {
            return BlendMapTexture{ .texture_descriptor = {
                .color = name,
                .mipmap_mode = MipmapMode::WITH_MIPMAPS } };
        }
    } else {
        return bit->second;
    }
}

void RenderingResources::set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt) {
    LOG_FUNCTION("RenderingResources::set_blend_map_texture " + name);
    std::scoped_lock lock{mutex_};
    if (!blend_map_textures_.insert({ name, bmt }).second) {
        THROW_OR_ABORT("Blend map texture with name \"" + name + "\" already exists");
    }
}

const FixedArray<double, 4, 4>& RenderingResources::get_vp(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_vp " + name);
    std::shared_lock lock{mutex_};
    auto it = vps_.find(name);
    if (it == vps_.end()) {
        THROW_OR_ABORT(
            "Could not find vp with name " + name + "."
            " Forgot to add a LightmapLogic for the light?"
            " Are dirtmaps enabled for the current scene?");
    }
    return it->second;
}

void RenderingResources::set_vp(const std::string& name, const FixedArray<double, 4, 4>& vp) {
    LOG_FUNCTION("RenderingResources::set_vp " + name);
    std::scoped_lock lock{mutex_};
    vps_[name] = vp;
}

float RenderingResources::get_offset(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    std::shared_lock lock{mutex_};
    auto it = offsets_.find(name);
    if (it == offsets_.end()) {
        THROW_OR_ABORT("Could not find offset with name " + name);
    }
    return it->second;
}

void RenderingResources::set_offset(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_offset " + name);
    std::scoped_lock lock{mutex_};
    offsets_[name] = value;
}

float RenderingResources::get_discreteness(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    std::shared_lock lock{mutex_};
    auto it = discreteness_.find(name);
    if (it == discreteness_.end()) {
        THROW_OR_ABORT("Could not find discreteness with name " + name);
    }
    return it->second;
}

void RenderingResources::set_discreteness(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_discreteness " + name);
    std::scoped_lock lock{mutex_};
    discreteness_[name] = value;
}

float RenderingResources::get_scale(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_scale " + name);
    std::shared_lock lock{mutex_};
    auto it = scales_.find(name);
    if (it == scales_.end()) {
        THROW_OR_ABORT("Could not find scale with name " + name);
    }
    return it->second;
}

void RenderingResources::set_scale(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_scale " + name);
    std::scoped_lock lock{mutex_};
    scales_[name] = value;
}

WrapMode RenderingResources::get_texture_wrap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_wrap " + name);
    std::shared_lock lock{mutex_};
    auto it = texture_wrap_.find(name);
    if (it == texture_wrap_.end()) {
        THROW_OR_ABORT("Could not find texture_wrap with name " + name);
    }
    return it->second;
}

void RenderingResources::set_texture_wrap(const std::string& name, WrapMode mode) {
    LOG_FUNCTION("RenderingResources::set_texture_wrap " + name);
    std::scoped_lock lock{mutex_};
    texture_wrap_[name] = mode;
}

void RenderingResources::delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode) {
    LOG_FUNCTION("RenderingResources::delete_vp " + name);
    std::scoped_lock lock{mutex_};
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
    std::scoped_lock lock{mutex_};
    if (textures_.erase(name) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            lwarn() << "Could not delete texture " << name;
        } else {
            THROW_OR_ABORT("Could not delete texture " + name);
        }
    }
}

std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& RenderingResources::render_programs() {
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
    {
        std::shared_lock lock{mutex_};
        auto it = font_textures_.find({ttf_filename, font_height_pixels});
        if (it != font_textures_.end()) {
            return it->second;
        }
    }
    std::scoped_lock lock{mutex_};
    {
        auto it = font_textures_.find({ttf_filename, font_height_pixels});
        if (it != font_textures_.end()) {
            return it->second;
        }
    }
    auto ins = font_textures_.insert({{ttf_filename, font_height_pixels}, LoadedFont()});
    if (!ins.second) {
        THROW_OR_ABORT("Could not insert font texture");
    }
    auto& result = ins.first->second;
    {
        const size_t TEXTURE_SIZE = 1024;
        std::vector<unsigned char> temp_bitmap(TEXTURE_SIZE * TEXTURE_SIZE);
        {
            std::vector<uint8_t> ttf_buffer = read_file_bytes(ttf_filename);
            // ASCII 32..126 is 95 glyphs
            result.cdata.resize(96);
            result.bottom_y = stbtt_BakeFontBitmap_get_y0(ttf_buffer.data(), 0, font_height_pixels, temp_bitmap.data(), TEXTURE_SIZE, TEXTURE_SIZE, 32, 96, result.cdata.data()); // no guarantee this fits!
            // can free ttf_buffer at this point
        }
        CHK(glGenTextures(1, &result.texture_handle));
        CHK(glBindTexture(GL_TEXTURE_2D, result.texture_handle));
        CHK(glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXTURE_SIZE, TEXTURE_SIZE, 0, GL_RED, GL_UNSIGNED_BYTE, temp_bitmap.data()));
        // can free temp_bitmap at this point
    }
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    return result;
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
        if (already_exists_behavior == TextureAlreadyExistsBehavior::IGNORE) {
            return;
        }
        THROW_OR_ABORT("DDS-texture with name \"" + name + "\" already exists");
    }
    if (preloaded_texture_data_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::IGNORE) {
            return;
        }
        THROW_OR_ABORT("Preloaded non-DDS-texture with name \"" + name + "\" already exists");
    }
    if (textures_.contains(name)) {
        if (already_exists_behavior == TextureAlreadyExistsBehavior::IGNORE) {
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
        if (!preloaded_texture_data_.try_emplace(name, std::move(image)).second) {
            THROW_OR_ABORT("Internal error: Preloaded STB-texture with name \"" + name + "\" already exists");
        }
    } else if (extension == ".dds") {
        if (!preloaded_texture_dds_data_.try_emplace(name, std::move(data)).second) {
            THROW_OR_ABORT("Internal error: Preloaded DDS-texture with name \"" + name + "\" already exists");
        }
    } else {
        THROW_OR_ABORT("Unknown file extension: \"" + name + '"');
    }
}

void RenderingResources::initialize_non_dds_texture(
    const std::string& name,
    const TextureDescriptor& descriptor) const
{
    StbInfo<uint8_t> si;
    if (preloaded_texture_data_.contains(name)) {
        if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
            linfo() << this << " Using preloaded texture: " << name;
        }
        si = std::move(preloaded_texture_data_.at(name));
        preloaded_texture_data_.erase(name);
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
    auto it = preloaded_texture_dds_data_.find(name);
    if (it == preloaded_texture_dds_data_.end()) {
        THROW_OR_ABORT("Could not find preloaded DDS-texture with name \"" + name + '"');
    }

    nv_dds::CDDSImage image;
    {
        std::stringstream sstr;
        for (uint8_t c : it->second) {
            sstr << c;
        }
        image.load(sstr);
    }

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
