#include "Rendering_Resources.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/CHK.hpp>
#include <cstring>
#include <memory>
#include <stb_image/stb_image_load.h>
#include <stb_image/stb_image_resize.h>
#include <string>
#include <vector>

using namespace Mlib;

static StbInfo stb_load_texture(const std::string& filename,
                                bool rgba,
                                bool flip_vertically,
                                bool flip_horizontally) {
    StbInfo result = stb_load(filename, flip_vertically, flip_horizontally);
    if (result.data == nullptr) {
        throw std::runtime_error("Could not load \"" + filename + '"');
    }
    if (result.nrChannels != 3 && result.nrChannels != 4) {
        throw std::runtime_error(filename + " does not have 3 or 4 channels");
    }
    if (rgba && (result.nrChannels == 3)) {
        throw std::runtime_error("Requested RGBA texture but it is RGB: " + filename);
    }
    return result;
}

RenderingResources::~RenderingResources() {
    for (const auto& t : textures_) {
        if (t.second.needs_gc) {
            CHK(glDeleteTextures(1, &t.second.handle));
        }
    }
}

// From: https://gamedev.stackexchange.com/questions/70829/why-is-gl-texture-max-anisotropy-ext-undefined/75816#75816?newreg=a7ddca6a76bf40b794c36dbe189c64b6
#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
#define GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT 0x84FF

GLuint RenderingResources::get_texture(const std::string& filename,
                                       bool rgba,
                                       const std::string& mixed,
                                       size_t overlap_npixels,
                                       const std::string& alias) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    auto it = textures_.find(TextureNameAndMixed{alias.empty() ? filename : alias, mixed});
    if (it == textures_.end()) {
        GLuint texture;

        CHK(glGenTextures(1, &texture));
        CHK(glBindTexture(GL_TEXTURE_2D, texture));
        {
            float aniso = 0.0f;
            CHK(glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &aniso));
            CHK(glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, aniso));
        }
        auto si0 = stb_load_texture(
            filename, rgba, true, false); // true=flip_vertically, false=flip_horizontally
        if (mixed != "") {
            auto si1_raw = stb_load_texture(
                mixed, rgba, true, false); // true=flip_vertically, false=flip_horizontally
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
        CHK(glTexImage2D(GL_TEXTURE_2D,
                         0,
                         rgba ? GL_RGBA : GL_RGB,
                         si0.width,
                         si0.height,
                         0,
                         si0.nrChannels == 3 ? GL_RGB : GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         si0.data.get()));
        CHK(glGenerateMipmap(GL_TEXTURE_2D));

        textures_.insert(
            std::make_pair(TextureNameAndMixed{alias.empty() ? filename : alias, mixed},
                           TextureHandleAndNeedsGc{texture, true}));
        return texture;
    }
    return it->second.handle;
}

GLuint RenderingResources::get_cubemap(const std::vector<std::string>& filenames,
                                       const std::string& alias) const {
    std::lock_guard<std::mutex> lock_guard{mutex_};
    auto it = textures_.find(TextureNameAndMixed{alias, ""});
    if (it == textures_.end()) {
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

        textures_.insert(std::make_pair(TextureNameAndMixed{alias, ""},
                                        TextureHandleAndNeedsGc{textureID, true}));
        return textureID;
    }
    return it->second.handle;
}

void RenderingResources::set_texture(const std::string& name, GLuint id) {
    // Old texture is deleted by frame buffer
    if (id == (GLuint)-1) {
        throw std::runtime_error("RenderingResources::set_texture: invalid texture ID");
    }
    textures_[TextureNameAndMixed{name, ""}] = TextureHandleAndNeedsGc{id, false};
}

const FixedArray<float, 4, 4>& RenderingResources::get_vp(const std::string& name) const {
    auto it = vps_.find(name);
    if (it == vps_.end()) {
        throw std::runtime_error("Could not find vp with name " + name);
    }
    return it->second;
}

void RenderingResources::set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp) {
    vps_[name] = vp;
}
