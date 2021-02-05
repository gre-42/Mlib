#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include <list>

namespace Mlib {

struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};
struct TextureDescriptor;
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class SceneNodeResources;
class RenderingResources;
struct BlendMapTexture;

enum class DeletionFailureMode {
    WARN,
    ERROR
};

class RenderingResources {
public:
    explicit RenderingResources(
        SceneNodeResources& scene_node_resources,
        const std::string& name,
        unsigned int max_anisotropic_filtering_level);
    ~RenderingResources();
    void preload(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const std::string& name, const TextureDescriptor& descriptor) const;
    GLuint get_normalmap_texture(const TextureDescriptor& descriptor) const;
    GLuint get_cubemap(const std::string& name, const std::vector<std::string>& filenames) const;
    void set_texture(const std::string& name, GLuint id);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);
    TextureDescriptor get_texture_descriptor(const std::string& name) const;

    BlendMapTexture get_blend_map_texture(const std::string& name) const;
    void set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt);

    const FixedArray<float, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp);
    float get_offset(const std::string& name) const;
    void set_offset(const std::string& name, float value);
    float get_discreteness(const std::string& name) const;
    void set_discreteness(const std::string& name, float value);
    WrapMode get_texture_wrap(const std::string& name) const;
    void set_texture_wrap(const std::string& name, WrapMode mode);

    void delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode);
    void delete_texture(const std::string& name, DeletionFailureMode deletion_failure_mode);

    std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    SceneNodeResources& scene_node_resources() const;
    const std::string& name() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

private:
    mutable std::map<std::string, TextureDescriptor> texture_descriptors_;
    mutable std::map<std::string, TextureHandleAndNeedsGc> textures_;
    mutable std::recursive_mutex mutex_;
    std::map<std::string, FixedArray<float, 4, 4>> vps_;
    std::map<std::string, float> offsets_;
    std::map<std::string, float> discreteness_;
    std::map<std::string, WrapMode> texture_wrap_;
    std::map<std::string, BlendMapTexture> blend_map_textures_;
    mutable std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    SceneNodeResources& scene_node_resources_;
    std::string name_;
    unsigned int max_anisotropic_filtering_level_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
