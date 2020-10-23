#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Material/Clamp_Mode.hpp>
#include <map>
#include <mutex>
#include <string>
#include <vector>

namespace Mlib {

struct TextureDescriptor;

struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};

class RenderingResources {
public:
    ~RenderingResources();
    GLuint get_texture(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const std::string& name, const TextureDescriptor& descriptor) const;
    GLuint get_cubemap(const std::string& name, const std::vector<std::string>& filenames) const;
    void set_texture(const std::string& name, GLuint id);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);

    const FixedArray<float, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp);
    float get_discreteness(const std::string& name) const;
    void set_discreteness(const std::string& name, float value);
    ClampMode get_texture_wrap(const std::string& name) const;
    void set_texture_wrap(const std::string& name, ClampMode mode);
private:
    mutable std::map<std::string, TextureDescriptor> texture_descriptors_;
    mutable std::map<std::string, TextureHandleAndNeedsGc> textures_;
    mutable std::mutex mutex_;
    std::map<std::string, FixedArray<float, 4, 4>> vps_;
    std::map<std::string, float> discreteness_;
    std::map<std::string, ClampMode> texture_wrap_;
};

}
