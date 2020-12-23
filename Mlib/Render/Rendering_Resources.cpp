#include "Rendering_Resources.hpp"
#include <Mlib/Geometry/Texture_Descriptor.hpp>
#include <Mlib/Images/Match_Rgba_Histograms.hpp>
#include <Mlib/Log.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Colored_Render_Program.hpp>
#include <iostream>
#include <memory>
#include <stb_image/stb_array.h>
#include <stb_image/stb_histogram.hpp>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_resize.h>
#include <stb_image/stb_mipmaps.h>
#include <string>
#include <vector>

using namespace Mlib;

/**
 * From: https://stackoverflow.com/questions/108318/whats-the-simplest-way-to-test-whether-a-number-is-a-power-of-2-in-c/108360#108360
 */
bool is_power_of_two(int n) {
    return n > 0 && ((n & (n - 1)) == 0);
}

/**
 * From: https://stackoverflow.com/questions/994593/how-to-do-an-integer-log2-in-c
 */
int log2(int n) {
    int result = 0;
    while (n >>= 1) ++result;
    return result;
}

static StbInfo stb_load_texture(const std::string& filename,
                                bool rgba,
                                bool flip_vertically,
                                bool flip_horizontally) {
    StbInfo result = stb_load(filename, flip_vertically, flip_horizontally);
    if (result.nrChannels != 3 && result.nrChannels != 4) {
        throw std::runtime_error(filename + " does not have 3 or 4 channels");
    }
    if (rgba && (result.nrChannels == 3)) {
        throw std::runtime_error("Requested RGBA texture but it is RGB: " + filename);
    }
    if (!is_power_of_two(result.width) || !is_power_of_two(result.height)) {
        std::cerr << filename << " size: " << result.width << 'x' << result.height << std::endl;
    }
    return result;
}

static void generate_rgba_mipmaps_inplace(const StbInfo& si) {
    if (!is_power_of_two(si.width) || !is_power_of_two(si.height)) {
        throw std::runtime_error("Image size is not a power of 2");
    }
    assert_true(si.nrChannels == 4);
    // assert_true(si.width > 0); // is contained in is_power_of_two
    // assert_true(si.height > 0); // is contained in is_power_of_two
    CHK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, log2(std::max(si.width, si.height))));

    // int w = si.width;
    // int h = si.height;
    int level = 0;
    // std::unique_ptr<unsigned char[]> si_resized{
    //     new unsigned char[(w / 2) * (h / 2) * si.nrChannels]};
    // unsigned char* cur_data = si.data.get();
    // unsigned char* resized_data = si_resized.get();
    RgbaDownsampler rds{si.data.get(), si.width, si.height};
    for (RgbaImage im = rds.next(); im.data != nullptr; im = rds.next()) {
        // if (level > 2) {
        //     VectorialPixels<float, 4> vp{ArrayShape{(size_t)im.width, (size_t)im.height}};
        //     std::transform(im.data, im.data + im.width * im.height * si.nrChannels, vp.flat_iterable().begin()->flat_begin(), [](unsigned char c){return c / 255.f;});
        //     Array<float> vpa = vp.to_array();
        //     // vpa[3] = gaussian_filter_NWE(vpa[3], 0.5f, float(NAN));
        //     // vpa[3] = (vpa[3] > 0.5f).casted<float>();
        //     float sigma = std::min(im.width, im.height) / 10.f;
        //     Array<float> m = gaussian_filter_NWE(vpa[3], sigma, float(NAN));
        //     for(size_t i = 0; i < 3; ++i) {
        //         vpa[i] = gaussian_filter_NWE(vpa[i] * vpa[3], sigma, float(NAN)) / m;
        //     }
        //     // static int ii = 0;
        //     // PgmImage::from_float(fi[3]).save_to_file("/tmp/alpha-" +  std::to_string(ii) + ".pgm");
        //     // PgmImage::from_float(vpa[3]).save_to_file("/tmp/alph0-" +  std::to_string(ii++) + ".pgm");
        //     VectorialPixels<float, 4> vpf{vpa};
        //     std::transform(vpf.flat_iterable().begin()->flat_begin(), vpf.flat_iterable().end()->flat_begin(), im.data, [](float f){return std::clamp(f * 255.f, 0.f, 255.f);});
        // }
        CHK(glTexImage2D(GL_TEXTURE_2D, level++, GL_RGBA, im.width, im.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, im.data));
        // if ((w > 1) && (h > 1)) {
        //     if (level > 2) {
        //         VectorialPixels<float, 4> vp{ArrayShape{(size_t)w, (size_t)h}};
        //         std::transform(cur_data, cur_data + w * h * si.nrChannels, vp.flat_iterable().begin()->flat_begin(), [](unsigned char c){return c / 255.f;});
        //         Array<float> vpa = vp.to_array();
        //         // vpa[3] = gaussian_filter_NWE(vpa[3], 0.5f, float(NAN));
        //         // vpa[3] = (vpa[3] > 0.5f).casted<float>();
        //         for(size_t i = 0; i < 3; ++i) {
        //             vpa[i] = gaussian_filter_NWE(vpa[i], 2.f, float(NAN));
        //         }
        //         // static int ii = 0;
        //         // PgmImage::from_float(fi[3]).save_to_file("/tmp/alpha-" +  std::to_string(ii) + ".pgm");
        //         // PgmImage::from_float(vpa[3]).save_to_file("/tmp/alph0-" +  std::to_string(ii++) + ".pgm");
        //         VectorialPixels<float, 4> vpf{vpa};
        //         std::transform(vpf.flat_iterable().begin()->flat_begin(), vpf.flat_iterable().end()->flat_begin(), cur_data, [](float f){return std::clamp(f * 255.f, 0.f, 255.f);});
        //     }
        //     stbir_resize_uint8(
        //         cur_data,
        //         w,
        //         h,
        //         0,
        //         resized_data,
        //         w / 2,
        //         h / 2,
        //         0,
        //         si.nrChannels);
        //     std::swap(cur_data, resized_data);
        //     w /= 2;
        //     h /= 2;
        //     ++level;
        // } else {
        //     break;
        // }
    }
    assert_true(level - 1 == log2(std::max(si.width, si.height)));
}

RenderingResources::RenderingResources() {}

RenderingResources::~RenderingResources() {
    for (const auto& t : textures_) {
        if (t.second.needs_gc) {
            CHK(glDeleteTextures(1, &t.second.handle));
        }
    }
}

GLuint RenderingResources::get_texture(const TextureDescriptor& descriptor) const {
    return get_texture(descriptor.color, descriptor);
}

// From: https://gamedev.stackexchange.com/questions/70829/why-is-gl-texture-max-anisotropy-ext-undefined/75816#75816?newreg=a7ddca6a76bf40b794c36dbe189c64b6
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

GLuint RenderingResources::get_texture(const std::string& name, const TextureDescriptor& descriptor) const {
    LOG_FUNCTION("RenderingResources::get_texture " + name);
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (auto it = textures_.find(name); it != textures_.end())
    {
        return it->second.handle;
    }
    auto dit = texture_descriptors_.find(name);
    const TextureDescriptor& desc = dit != texture_descriptors_.end()
        ? dit->second
        : descriptor;
    
    GLuint texture;

    CHK(glGenTextures(1, &texture));
    CHK(glBindTexture(GL_TEXTURE_2D, texture));
    {
        float aniso = 0.0f;
        CHK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
        CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
    }
    StbInfo si0 = stb_load_texture(
        desc.color, desc.color_mode == ColorMode::RGBA, true, false); // true=flip_vertically, false=flip_horizontally
    if (!desc.mixed.empty()) {
        auto si1_raw = stb_load_texture(
            desc.mixed, desc.color_mode == ColorMode::RGBA, true, false); // true=flip_vertically, false=flip_horizontally
        std::unique_ptr<unsigned char[]> si1_resized{
            new unsigned char[si0.width * si0.height * si1_raw.nrChannels]};
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
                        fac * si0.data.get()[i0] + (1 - fac) * si1_resized.get()[i1];
                }
            }
        }
    }
    if (!desc.mean_color.all_equal(-1.f)) {
        if (!stb_match_color_rgb(
            si0.data.get(),
            si0.width,
            si0.height,
            si0.nrChannels,
            (desc.mean_color * 255.f).casted<unsigned char>().flat_begin()))
        {
            std::cerr << "alpha = 0: " << desc.color << std::endl;
        }
    }
    if (!desc.histogram.empty()) {
        Array<unsigned char> image = stb_image_2_array(si0);
        Array<unsigned char> ref = stb_image_2_array(stb_load_texture(desc.histogram, false, false, false));
        Array<unsigned char> m = match_rgba_histograms(image, ref);
        assert_true(m.shape(0) == (size_t)si0.nrChannels);
        assert_true(m.shape(1) == (size_t)si0.height);
        assert_true(m.shape(2) == (size_t)si0.width);
        array_2_stb_image(m, si0.data.get());
    }
    CHK(glTexImage2D(GL_TEXTURE_2D,
                     0,
                     desc.color_mode == ColorMode::RGBA ? GL_RGBA : GL_RGB,
                     si0.width,
                     si0.height,
                     0,
                     si0.nrChannels == 3 ? GL_RGB : GL_RGBA,
                     GL_UNSIGNED_BYTE,
                     si0.data.get()));
    if (desc.color_mode == ColorMode::RGBA) {
        generate_rgba_mipmaps_inplace(si0);
    } else {
        CHK(glGenerateMipmap(GL_TEXTURE_2D));
    }
    // CHK(glGenerateMipmap(GL_TEXTURE_2D));

    textures_.insert({name, TextureHandleAndNeedsGc{texture, true}});
    return texture;
}

GLuint RenderingResources::get_cubemap(const std::string& name,
                                       const std::vector<std::string>& filenames) const
{
    LOG_FUNCTION("RenderingResources::get_cubemap " + name);
    std::lock_guard<std::mutex> lock_guard{mutex_};
    if (auto it = textures_.find(name); it != textures_.end()) {
        return it->second.handle;
    }
    if (filenames.size() != 6) {
        throw std::runtime_error("Cubemap filenames do not have length 6");
    }
    GLuint textureID;
    CHK(glGenTextures(1, &textureID));
    CHK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureID));

    for (GLuint i = 0; i < filenames.size(); i++) {
        StbInfo info =
            stb_load_texture(filenames[i],
                                false,
                                false,
                                true); // false=rgba, false=flip_vertically, true=flip_horizontally
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
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        throw std::runtime_error("RenderingResources::set_texture: invalid texture ID");
    }
    textures_[name] = TextureHandleAndNeedsGc{id, false};
}

void RenderingResources::add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor)
{
    LOG_FUNCTION("RenderingResources::add_texture_descriptor " + name);
    if (auto it = texture_descriptors_.insert({name, descriptor}); !it.second) {
        throw std::runtime_error("Texture descriptor with name " + name + " already exists");
    }
}

std::string RenderingResources::get_normalmap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::has_normalmap " + name);
    if (auto it = texture_descriptors_.find(name); it == texture_descriptors_.end()) {
        return "";
    } else {
        return it->second.normal;
    }
}

const FixedArray<float, 4, 4>& RenderingResources::get_vp(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_vp " + name);
    auto it = vps_.find(name);
    if (it == vps_.end()) {
        throw std::runtime_error("Could not find vp with name " + name + ". Forgot to add a LightmapLogic for the light?");
    }
    return it->second;
}

void RenderingResources::set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp) {
    LOG_FUNCTION("RenderingResources::set_vp " + name);
    vps_[name] = vp;
}

float RenderingResources::get_discreteness(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_discreteness " + name);
    auto it = discreteness_.find(name);
    if (it == discreteness_.end()) {
        throw std::runtime_error("Could not find discreteness with name " + name);
    }
    return it->second;
}

void RenderingResources::set_discreteness(const std::string& name, float value) {
    LOG_FUNCTION("RenderingResources::set_discreteness " + name);
    discreteness_[name] = value;
}

WrapMode RenderingResources::get_texture_wrap(const std::string& name) const {
    LOG_FUNCTION("RenderingResources::get_texture_wrap " + name);
    auto it = texture_wrap_.find(name);
    if (it == texture_wrap_.end()) {
        throw std::runtime_error("Could not find texture_wrap with name " + name);
    }
    return it->second;
}

void RenderingResources::set_texture_wrap(const std::string& name, WrapMode mode) {
    LOG_FUNCTION("RenderingResources::set_texture_wrap " + name);
    texture_wrap_[name] = mode;
}

void RenderingResources::delete_vp(const std::string& name) {
    if (vps_.erase(name) != 1) {
        throw std::runtime_error("Could not delete VP " + name);
    }
}
void RenderingResources::delete_texture(const std::string& name) {
    if (textures_.erase(name) != 1) {
        throw std::runtime_error("Could not delete texture " + name);
    }
}

std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& RenderingResources::render_programs() {
    return render_programs_;
}
