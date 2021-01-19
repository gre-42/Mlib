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

struct TextureDescriptor;
struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class SceneNodeResources;
class RenderingResources;

class RenderingResourcesGuard {
public:
    explicit RenderingResourcesGuard(SceneNodeResources& scene_node_resources);
    explicit RenderingResourcesGuard(const std::shared_ptr<RenderingResources>& rr);
    ~RenderingResourcesGuard();
};

class RenderingResources {
public:
    explicit RenderingResources(SceneNodeResources& scene_node_resources);
    ~RenderingResources();
    void preload(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const std::string& name, const TextureDescriptor& descriptor) const;
    GLuint get_cubemap(const std::string& name, const std::vector<std::string>& filenames) const;
    void set_texture(const std::string& name, GLuint id);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);
    std::string get_normalmap(const std::string& name) const;

    const FixedArray<float, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<float, 4, 4>& vp);
    float get_discreteness(const std::string& name) const;
    void set_discreteness(const std::string& name, float value);
    WrapMode get_texture_wrap(const std::string& name) const;
    void set_texture_wrap(const std::string& name, WrapMode mode);

    void delete_vp(const std::string& name);
    void delete_texture(const std::string& name);

    std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    SceneNodeResources& scene_node_resources() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

    static std::shared_ptr<RenderingResources> primary_rendering_resources();
    static std::shared_ptr<RenderingResources> rendering_resources();
    static thread_local std::list<std::shared_ptr<RenderingResources>> rendering_resources_stack_;
    static std::function<std::function<void()>(std::function<void()>)>
        generate_thread_runner(
            std::shared_ptr<RenderingResources> rendering_resources);
    static std::function<std::function<void()>(std::function<void()>)>
        generate_thread_runner(
            std::shared_ptr<RenderingResources> primary_rendering_resources,
            std::shared_ptr<RenderingResources> secondary_rendering_resources);
    static void print_stack(std::ostream& ostr);
private:
    mutable std::map<std::string, TextureDescriptor> texture_descriptors_;
    mutable std::map<std::string, TextureHandleAndNeedsGc> textures_;
    mutable std::recursive_mutex mutex_;
    std::map<std::string, FixedArray<float, 4, 4>> vps_;
    std::map<std::string, float> discreteness_;
    std::map<std::string, WrapMode> texture_wrap_;
    mutable std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    SceneNodeResources& scene_node_resources_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
