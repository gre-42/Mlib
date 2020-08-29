#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/String.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace Mlib {

struct TextureNameAndMixed {
    std::string name;
    std::string mixed;
    std::strong_ordering operator <=> (const TextureNameAndMixed&) const = default;
};

struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};

class RenderingResources {
public:
    ~RenderingResources();
    GLuint get_texture(
        const std::string& filename,
        bool rgba,
        const std::string& mixed = "",
        size_t overlap_npixels = 5,
        const std::string& alias = "") const;
    GLuint get_cubemap(const std::vector<std::string>& filenames, const std::string& alias) const;
    void set_texture(const std::string& name, GLuint id);

    const FixedArray<float, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp);
private:
    mutable std::map<TextureNameAndMixed, TextureHandleAndNeedsGc> textures_;
    mutable std::mutex mutex_;
    std::map<std::string, FixedArray<float, 4, 4>> vps_;
};

}
