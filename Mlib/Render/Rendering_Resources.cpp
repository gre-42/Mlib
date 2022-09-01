#include "Rendering_Resources.hpp"
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Material/Blend_Map_Texture.hpp>
#include <Mlib/Geometry/Material/Texture_Descriptor.hpp>
#include <Mlib/Images/Extrapolate_Rgba_Colors.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Images/StbImage4.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Is_Power_Of_Two.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>
#include <filesystem>
#include <iostream>
#include <memory>
#include <mutex>
#include <stb_image/stb_array.h>
#include <stb_image/stb_colorize.hpp>
#include <stb_image/stb_desaturate.hpp>
#include <stb_image/stb_image_atlas.hpp>
#include <stb_image/stb_image_load.hpp>
#include <stb_image/stb_image_resize.h>
#include <stb_image/stb_image_write.h>
#include <stb_image/stb_lighten.hpp>
#include <stb_image/stb_mipmaps.h>
#include <stb_image/stb_set_alpha.hpp>
#include <string>
#include <vector>

using namespace Mlib;
namespace fs = std::filesystem;

/**
 * From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
 */
int log2(int n) {
    int result = 0;
    while (n >>= 1) ++result;
    return result;
}

static StbInfo stb_load_texture(const std::string& filename,
                                int nchannels,
                                bool flip_vertically,
                                bool flip_horizontally) {
    StbInfo result = stb_load(filename, flip_vertically, flip_horizontally);
    if (result.nrChannels < std::abs(nchannels)) {
        throw std::runtime_error(filename + " does not have at least " + std::to_string(nchannels) + " channels");
    }
    if (!is_power_of_two(result.width) || !is_power_of_two(result.height)) {
        std::cerr << filename << " size: " << result.width << 'x' << result.height << std::endl;
    }
    if ((nchannels > 0) && (result.nrChannels != nchannels)) {
        std::cerr << filename << " #channels: " << result.nrChannels << std::endl;
    }
    return result;
}

static StbInfo stb_load_and_transform_texture(const TextureDescriptor& desc) {
    std::string touch_file = desc.color + ".xpltd";
    if ((desc.color_mode == ColorMode::RGBA) && desc.alpha.empty() && !fs::exists(touch_file)) {
        std::cerr << "Extrapolating RGBA image \"" << desc.color << '"' << std::endl;
        auto img = StbImage4::load_from_file(desc.color);
        float sigma = 3.f;
        size_t niterations = 1 + std::max(img.shape(0), img.shape(1)) / (sigma * 4);
        extrapolate_rgba_colors(
            img,
            sigma,
            niterations).save_to_file(desc.color);
        std::ofstream ofstr{ touch_file };
        if (ofstr.fail()) {
            throw std::runtime_error("Could not create file \"" + touch_file + '"');
        }
    }
    StbInfo si0;
    if (!desc.alpha.empty()) {
        if (desc.color_mode != ColorMode::RGBA) {
            throw std::runtime_error("Color mode not RGBA despite alpha texture: \"" + desc.color + '"');
        }
        si0 = stb_load_texture(
            desc.color, (int)ColorMode::RGB, true, false); // true=flip_vertically, false=flip_horizontally
        if (si0.nrChannels != 3) {
            throw std::runtime_error("#channels not 3: \"" + desc.color + '"');
        }
        auto si_alpha = stb_load_texture(
            desc.alpha, (int)ColorMode::GRAYSCALE, true, false); // true=flip_vertically, false=flip_horizontally
        if (si_alpha.nrChannels != 1) {
            throw std::runtime_error("#channels not 1: \"" + desc.alpha + '"');
        }
        if ((si_alpha.width != si0.width) ||
            (si_alpha.height != si0.height))
        {
            throw std::runtime_error("Size mismatch between files \"" + desc.color + "\" and \"" + desc.alpha + '"');
        }
        StbInfo si0_rgb = std::move(si0);
        si0 = stb_create(si0.width, si0.height, 4);
        stb_set_alpha(
            si0_rgb.data.get(),
            si_alpha.data.get(),
            si0.data.get(),
            si0.width,
            si0.height);
    } else {
        si0 = stb_load_texture(
            desc.color, (int)desc.color_mode, true, false); // true=flip_vertically, false=flip_horizontally
    }
    if (!desc.mixed.empty()) {
        auto si1_raw = stb_load_texture(
            desc.mixed, (int)desc.color_mode, true, false); // true=flip_vertically, false=flip_horizontally
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
                    fac = float(dist) / max_dist;
                } else {
                    fac = 1;
                }
                for (int d = 0; d < si0.nrChannels; ++d) {
                    int i0 = (r * si0.width + c) * si0.nrChannels + d;
                    int i1 = (r * si0.width + c) * si1_raw.nrChannels + d;
                    si0.data.get()[i0] =
                        (unsigned char)(fac * si0.data.get()[i0] + (1 - fac) * si1_resized.get()[i1]);
                }
            }
        }
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
        Array<unsigned char> ref = stb_image_2_array(stb_load_texture(desc.histogram, -3, false, false));
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
            !all(isfinite(lighten)))
        {
            throw std::runtime_error("Lighten value out of bounds");
        }
        stb_lighten(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.lighten * 255.f).casted<short>().flat_begin());
    }
    if (!desc.mean_color.all_equal(-1.f)) {
        if (!stb_colorize(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            std::cerr << "alpha = 0: " << desc.color << std::endl;
        }
    }
    return si0;
}

// static void generate_rgba_mipmaps_inplace(const StbInfo& si) {
//     if (!is_power_of_two(si.width) || !is_power_of_two(si.height)) {
//         throw std::runtime_error("Image size is not a power of 2");
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
    const std::string& name,
    unsigned int max_anisotropic_filtering_level)
: name_{ name },
  max_anisotropic_filtering_level_{ max_anisotropic_filtering_level }
{}

RenderingResources::~RenderingResources() {
    for (const auto& t : textures_) {
        if (t.second.needs_gc) {
            gc_textures.push_back(t.second.handle);
        }
    }
}

void RenderingResources::preload(const TextureDescriptor& descriptor) const {
    LOG_FUNCTION("RenderingResources::preload, color=" + descriptor.color);
    std::unique_lock lock{ mutex_ };
    auto dit = texture_descriptors_.find(descriptor.color);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    if (glfwGetCurrentContext() != nullptr) {
        if (!desc.color.empty()) {
            get_texture(desc);
        }
        if (!desc.normal.empty()) {
            get_normalmap_texture(desc);
        }
    } else {
        if (!desc.color.empty() && !textures_.contains(desc.color) && !preloaded_texture_data_.contains(desc.color)) {
            if (!preloaded_texture_data_.insert({desc.color, get_texture_data(desc)}).second) {
                throw std::runtime_error("Could not preload color");
            }
        }
        if (!desc.normal.empty() && !textures_.contains(desc.normal) && !preloaded_texture_data_.contains(desc.normal)) {
            if (!preloaded_texture_data_.insert({
                desc.normal,
                get_texture_data(TextureDescriptor{
                    .color = desc.normal,
                    .color_mode = ColorMode::RGB})}).second)
            {
                throw std::runtime_error("Could not preload normal");
            }
        }
    }
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
    
    if (atlas_tile_descriptors_.contains(desc.color) ||
        desc.desaturate ||
        !desc.histogram.empty() ||
        !desc.mixed.empty() ||
        !desc.mean_color.all_equal(-1.f) ||
        !desc.lighten.all_equal(0.f))
    {
        StbInfo si = get_texture_data(desc);
        if (!default_filename.ends_with(".png")) {
            throw std::runtime_error("Filename \"" + default_filename + "\" does not end with .png");
        }
        if (!stbi_write_png(
            default_filename.c_str(),
            si.width,
            si.height,
            si.nrChannels,
            si.data.get(),
            0))
        {
            throw std::runtime_error("Could not save to file \"" + default_filename + '"');
        }
        return default_filename;
    } else {
        return desc.color;
    }
}
    
GLuint RenderingResources::get_texture(const TextureDescriptor& descriptor) const {
    return get_texture(descriptor.color, descriptor);
}

GLuint RenderingResources::get_normalmap_texture(const TextureDescriptor& descriptor) const {
    return get_texture(TextureDescriptor{
        .color = descriptor.normal,
        .color_mode = ColorMode::RGB,
        .anisotropic_filtering_level = descriptor.anisotropic_filtering_level});
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
        throw std::runtime_error("Unsupported number of channels: " + std::to_string(nchannels));
    };
}

// From: https://gamedev.stackexchange.com/questions/70829/why-is-gl-texture-max-anisotropy-ext-undefined/75816#75816?newreg=a7ddca6a76bf40b794c36dbe189c64b6
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

GLuint RenderingResources::get_texture(const std::string& name, const TextureDescriptor& descriptor) const {
    LOG_FUNCTION("RenderingResources::get_texture " + name);
    std::unique_lock lock{mutex_};
    if (auto it = textures_.find(name); it != textures_.end())
    {
        return it->second.handle;
    }
    auto dit = texture_descriptors_.find(name);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    if (getenv_default_bool("PRINT_TEXTURE_FILENAMES", false)) {
        std::cout << "Loading texture: " << desc.color << std::endl;
        if (!desc.mixed.empty()) {
            std::cout << "Loading texture: " << desc.mixed << std::endl;
        }
    }
    
    GLuint texture;

    CHK(glGenTextures(1, &texture));
    CHK(glBindTexture(GL_TEXTURE_2D, texture));
    if ((desc.anisotropic_filtering_level != 0) &&
        (max_anisotropic_filtering_level_ != 0))
    {
        float aniso = 0.0f;
        CHK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
        aniso = std::min({ aniso, (float)desc.anisotropic_filtering_level, float(max_anisotropic_filtering_level_) });
        if (aniso != 0) {
            CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
    }
    StbInfo si;
    if (preloaded_texture_data_.contains(name)) {
        si = std::move(preloaded_texture_data_.at(name));
        preloaded_texture_data_.erase(name);
    } else {
        si = get_texture_data(desc);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);  // https://stackoverflow.com/a/49126350/2292832
    CHK(glTexImage2D(GL_TEXTURE_2D,
                     0,
                     nchannels2format(size_t(desc.color_mode)),
                     si.width,
                     si.height,
                     0,
                     nchannels2format(si.nrChannels),
                     GL_UNSIGNED_BYTE,
                     si.data.get()));
    // if (si.nrChannels == 4) {
    //     generate_rgba_mipmaps_inplace(si);
    // } else {
    //     CHK(glGenerateMipmap(GL_TEXTURE_2D));
    // }
    CHK(glGenerateMipmap(GL_TEXTURE_2D));

    textures_.insert({name, TextureHandleAndNeedsGc{texture, true}});
    return texture;
}

GLuint RenderingResources::get_cubemap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_cubemap " + name);
    std::unique_lock lock{mutex_};
    if (auto it = textures_.find(name); it != textures_.end()) {
        return it->second.handle;
    }
    auto it = cubemap_descriptors_.find(name);
    if (it == cubemap_descriptors_.end()) {
        throw std::runtime_error("Could not find cubemap \"" + name + '"');
    }
    if (it->second.filenames.size() != 6) {
        throw std::runtime_error("Cubemap does not have 6 filenames");
    }
    GLuint textureID;
    CHK(glGenTextures(1, &textureID));
    CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    for (GLuint i = 0; i < it->second.filenames.size(); i++) {
        StbInfo info =
            stb_load_texture(it->second.filenames[i],
                             3,       // nchannels
                             false,   // false=flip_vertically
                             false);  // false=flip_horizontally
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

    if (auto it = textures_.insert({name, TextureHandleAndNeedsGc{textureID, true}}); !it.second) {
        throw std::runtime_error("Cubemap with name " + name + " already exists");
    }
    return textureID;
}

void RenderingResources::set_texture(const std::string& name, GLuint id) {
    LOG_FUNCTION("RenderingResources::set_texture " + name);
    std::unique_lock lock{mutex_};
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        throw std::runtime_error("RenderingResources::set_texture: invalid texture ID");
    }
    textures_[name] = TextureHandleAndNeedsGc{id, false};
}

void RenderingResources::add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + name);
    std::unique_lock lock{mutex_};
    if (auto it = texture_descriptors_.insert({name, descriptor}); !it.second) {
        throw std::runtime_error("Texture descriptor with name " + name + " already exists");
    }
}

TextureDescriptor RenderingResources::get_existing_texture_descriptor(const std::string& name) const {
    std::shared_lock lock{mutex_};
    auto it = texture_descriptors_.find(name);
    if (it == texture_descriptors_.end()) {
        throw std::runtime_error("Could not find texture descriptor: " + name);
    }
    return it->second;
}

void RenderingResources::add_texture_atlas(
    const std::string& name,
    const TextureAtlasDescriptor& texture_atlas_descriptor)
{
    std::unique_lock lock{mutex_};
    if (auto it = atlas_tile_descriptors_.insert({name, texture_atlas_descriptor}); !it.second) {
        throw std::runtime_error("Atlas descriptor with name " + name + " already exists");
    } 
}

void RenderingResources::add_cubemap(const std::string& name, const std::vector<std::string>& filenames, bool desaturate) {
    std::unique_lock lock{mutex_};
    auto it = textures_.find(name);
    if (it != textures_.end()) {
        throw std::runtime_error("Texture with name \"" + name + "\" already exists");
    }
    if (!cubemap_descriptors_.insert({
        name,
        CubemapDescriptor{
            .filenames = filenames,
            .desaturate = desaturate}}).second)
    {
        throw std::runtime_error("Cubemap with name \"" + name + "\" already exists");
    }
}

StbInfo RenderingResources::get_texture_data(const TextureDescriptor& descriptor) const {
    if (auto it = atlas_tile_descriptors_.find(descriptor.color); it != atlas_tile_descriptors_.end()) {
        StbInfo si = stb_create(it->second.width, it->second.height, (int)it->second.color_mode);
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
                .image = get_texture_data(desc)});
        }
        build_image_atlas(si, atlas_tiles);
        return si;
    }
    return stb_load_and_transform_texture(descriptor);
}

BlendMapTexture RenderingResources::get_blend_map_texture(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_blending_min " + name);
    std::shared_lock lock{mutex_};
    if (auto bit = blend_map_textures_.find(name); bit == blend_map_textures_.end()) {
        if (auto tit = texture_descriptors_.find(name); tit != texture_descriptors_.end()) {
            return BlendMapTexture{ .texture_descriptor = {
                .color = name,
                .alpha = tit->second.alpha,
                .specular = tit->second.specular,
                .normal = tit->second.normal } };
        } else {
            return BlendMapTexture{ .texture_descriptor = { .color = name } };
        }
    } else {
        return bit->second;
    }
}

void RenderingResources::set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt) {
    std::unique_lock lock{mutex_};
    if (!blend_map_textures_.insert({ name, bmt }).second) {
        throw std::runtime_error("Blend map texture with name \"" + name + "\" already exists");
    }
}

const FixedArray<double, 4, 4>& RenderingResources::get_vp(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_vp " + name);
    std::shared_lock lock{mutex_};
    auto it = vps_.find(name);
    if (it == vps_.end()) {
        throw std::runtime_error(
            "Could not find vp with name " + name + "."
            " Forgot to add a LightmapLogic for the light?"
            " Are dirtmaps enabled for the current scene?");
    }
    return it->second;
}

void RenderingResources::set_vp(const std::string& name, const FixedArray<double, 4, 4>& vp) {
    LOG_FUNCTION("RenderingResources::set_vp " + name);
    std::unique_lock lock{mutex_};
    vps_[name] = vp;
}

float RenderingResources::get_offset(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    std::shared_lock lock{mutex_};
    auto it = offsets_.find(name);
    if (it == offsets_.end()) {
        throw std::runtime_error("Could not find offset with name " + name);
    }
    return it->second;
}

void RenderingResources::set_offset(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_offset " + name);
    std::unique_lock lock{mutex_};
    offsets_[name] = value;
}

float RenderingResources::get_discreteness(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    std::shared_lock lock{mutex_};
    auto it = discreteness_.find(name);
    if (it == discreteness_.end()) {
        throw std::runtime_error("Could not find discreteness with name " + name);
    }
    return it->second;
}

void RenderingResources::set_discreteness(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_discreteness " + name);
    std::unique_lock lock{mutex_};
    discreteness_[name] = value;
}

float RenderingResources::get_scale(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_scale " + name);
    std::shared_lock lock{mutex_};
    auto it = scales_.find(name);
    if (it == scales_.end()) {
        throw std::runtime_error("Could not find scale with name " + name);
    }
    return it->second;
}

void RenderingResources::set_scale(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_scale " + name);
    std::unique_lock lock{mutex_};
    scales_[name] = value;
}

WrapMode RenderingResources::get_texture_wrap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_wrap " + name);
    std::shared_lock lock{mutex_};
    auto it = texture_wrap_.find(name);
    if (it == texture_wrap_.end()) {
        throw std::runtime_error("Could not find texture_wrap with name " + name);
    }
    return it->second;
}

void RenderingResources::set_texture_wrap(const std::string& name, WrapMode mode) {
    LOG_FUNCTION("RenderingResources::set_texture_wrap " + name);
    std::unique_lock lock{mutex_};
    texture_wrap_[name] = mode;
}

void RenderingResources::delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode) {
    std::unique_lock lock{mutex_};
    if (vps_.erase(name) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            std::cerr << "WARNING: Could not delete VP " << name << std::endl;
        } else {
            throw std::runtime_error("Could not delete VP " + name);
        }
    }
}
void RenderingResources::delete_texture(const std::string& name, DeletionFailureMode deletion_failure_mode) {
    std::unique_lock lock{mutex_};
    if (textures_.erase(name) != 1) {
        if (deletion_failure_mode == DeletionFailureMode::WARN) {
            std::cerr << "WARNING: Could not delete texture " << name << std::endl;
        } else {
            throw std::runtime_error("Could not delete texture " + name);
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
