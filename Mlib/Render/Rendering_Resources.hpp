#pragma once
#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <list>

struct StbInfo;

namespace Mlib {

struct TextureDescriptor;
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class RenderingResources;
struct BlendMapTexture;
enum class ColorMode;

struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};

struct AtlasTileDescriptor {
    int left;
    int bottom;
    std::string filename;
};

struct TextureAtlasDescriptor {
    int width;
    int height;
    ColorMode color_mode;
    std::vector<AtlasTileDescriptor> tiles;
};

struct CubemapDescriptor {
    std::vector<std::string> filenames;
    bool desaturate;
};

enum class DeletionFailureMode {
    WARN,
    ERROR
};

class RenderingResources {
public:
    explicit RenderingResources(
        std::string name,
        unsigned int max_anisotropic_filtering_level);
    ~RenderingResources();
    void preload(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const TextureDescriptor& descriptor) const;
    GLuint get_texture(const std::string& name, const TextureDescriptor& descriptor) const;
    GLuint get_normalmap_texture(const TextureDescriptor& descriptor) const;
    GLuint get_cubemap(const std::string& name) const;
    bool contains_texture(const std::string& name) const;
    void set_texture(const std::string& name, GLuint id);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);
    TextureDescriptor get_existing_texture_descriptor(const std::string& name) const;
    void add_texture_atlas(const std::string& name, const TextureAtlasDescriptor& texture_atlas_descriptor);
    void add_cubemap(const std::string& name, const std::vector<std::string>& filenames, bool desaturate);

    std::string get_texture_filename(
        const TextureDescriptor& descriptor,
        const std::string& default_filename) const;

    BlendMapTexture get_blend_map_texture(const std::string& name) const;
    void set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt);

    const FixedArray<double, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<double, 4, 4>& vp);
    float get_offset(const std::string& name) const;
    void set_offset(const std::string& name, float value);
    float get_discreteness(const std::string& name) const;
    void set_discreteness(const std::string& name, float value);
    float get_scale(const std::string& name) const;
    void set_scale(const std::string& name, float value);
    WrapMode get_texture_wrap(const std::string& name) const;
    void set_texture_wrap(const std::string& name, WrapMode mode);

    void delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode);
    void delete_texture(const std::string& name, DeletionFailureMode deletion_failure_mode);

    std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    const std::string& name() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

    StbInfo get_texture_data(const TextureDescriptor& descriptor) const;

private:
    mutable std::map<std::string, StbInfo> preloaded_texture_data_;
    mutable std::map<std::string, TextureDescriptor> texture_descriptors_;
    mutable std::map<std::string, TextureHandleAndNeedsGc> textures_;
    mutable std::map<std::string, TextureAtlasDescriptor> atlas_tile_descriptors_;
    mutable std::map<std::string, CubemapDescriptor> cubemap_descriptors_;
    mutable RecursiveSharedMutex mutex_;
    std::map<std::string, FixedArray<double, 4, 4>> vps_;
    std::map<std::string, float> offsets_;
    std::map<std::string, float> discreteness_;
    std::map<std::string, float> scales_;
    std::map<std::string, WrapMode> texture_wrap_;
    std::map<std::string, BlendMapTexture> blend_map_textures_;
    mutable std::map<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    std::string name_;
    unsigned int max_anisotropic_filtering_level_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
