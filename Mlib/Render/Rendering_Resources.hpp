#pragma once
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Map/Threadsafe_Map.hpp>
#include <Mlib/Map/Threadsafe_String_Map.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <cstdint>
#include <functional>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

template <class TData>
struct StbInfo;
enum class FlipMode;

namespace Mlib {

struct ColormapWithModifiers;
struct TextureDescriptor;
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class RenderingResources;
struct BlendMapTexture;
enum class ColorMode;
struct LoadedFont;

struct TextureHandleAndNeedsGc {
    GLuint handle;
    bool needs_gc;
};

struct ManualAtlasTileDescriptor {
    int left;
    int bottom;
    std::string filename;
};

struct AutoAtlasTileDescriptor {
    int left;
    int bottom;
    int width;
    int height;
    std::string filename;
};

std::ostream& operator << (std::ostream& ostr, const AutoAtlasTileDescriptor& aatd);

struct ManualTextureAtlasDescriptor {
    int width;
    int height;
    ColorMode color_mode;
    std::vector<ManualAtlasTileDescriptor> tiles;
};

struct AutoTextureAtlasDescriptor {
    int width;
    int height;
    int mip_level_count;
    ColorMode color_mode;
    std::list<std::vector<AutoAtlasTileDescriptor>> tiles;
};

std::ostream& operator << (std::ostream& ostr, const AutoTextureAtlasDescriptor& atad);

struct CubemapDescriptor {
    std::vector<std::string> filenames;
};

enum class DeletionFailureMode {
    WARN,
    ERROR
};

enum class CallerType {
    PRELOAD,
    RENDER
};

class RenderingResources final: public IDdsResources {
    RenderingResources(const RenderingResources&) = delete;
    RenderingResources& operator = (const RenderingResources&) = delete;
public:
    explicit RenderingResources(
        std::string name,
        unsigned int max_anisotropic_filtering_level);
    ~RenderingResources();
    void preload(const TextureDescriptor& descriptor) const;
    bool texture_is_loaded_and_try_preload(const TextureDescriptor& descriptor);
    GLuint get_texture(
        const TextureDescriptor& descriptor,
        CallerType caller_type = CallerType::RENDER) const;
    GLuint get_texture(
        const ColormapWithModifiers& name,
        const TextureDescriptor& descriptor,
        CallerType caller_type = CallerType::RENDER) const;
    GLuint get_normalmap_texture(const TextureDescriptor& descriptor) const;
    GLuint get_cubemap(const std::string& name) const;
    bool contains_texture(const ColormapWithModifiers& name) const;
    void set_texture(const std::string& name, GLuint id);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);
    TextureDescriptor get_existing_texture_descriptor(const std::string& name) const;
    void add_manual_texture_atlas(const std::string& name, const ManualTextureAtlasDescriptor& texture_atlas_descriptor);
    std::map<std::string, ManualUvTile> generate_manual_texture_atlas(
        const std::string& name,
        const std::vector<std::string>& filenames);
    std::map<std::string, AutoUvTile> generate_auto_texture_atlas(
        const std::string& name,
        const std::vector<std::string>& filenames,
        AutoTextureAtlasDescriptor* atlas = nullptr);
    void add_cubemap(const std::string& name, const std::vector<std::string>& filenames);

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

    ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    const std::string& name() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

    StbInfo<uint8_t> get_texture_data(const TextureDescriptor& descriptor, FlipMode flip_mode) const;

    const LoadedFont& get_font_texture(const std::string& ttf_filename, float font_height_pixels) const;

    void save_to_file(const std::string& filename, const TextureDescriptor& desc) const;

    virtual void insert_texture(
        const std::string& name,
        std::vector<uint8_t>&& data,
        TextureAlreadyExistsBehavior already_exists_behavior) override;

private:
    bool texture_is_loaded_unsafe(const ColormapWithModifiers& name) const;
    void deallocate();
    void initialize_non_dds_texture(const ColormapWithModifiers& name, const TextureDescriptor& descriptor) const;
    void initialize_dds_texture(const std::string& name, const TextureDescriptor& descriptor) const;
    void add_auto_texture_atlas(const std::string& name, const AutoTextureAtlasDescriptor& texture_atlas_descriptor);
    mutable SafeRecursiveSharedMutex mutex_;
    mutable ThreadsafeMap<ColormapWithModifiers, StbInfo<uint8_t>> preloaded_texture_data_;
    mutable ThreadsafeStringMap<std::vector<uint8_t>> preloaded_texture_dds_data_;
    mutable ThreadsafeStringMap<TextureDescriptor> texture_descriptors_;
    mutable ThreadsafeMap<ColormapWithModifiers, TextureHandleAndNeedsGc> textures_;
    mutable ThreadsafeStringMap<ManualTextureAtlasDescriptor> manual_atlas_tile_descriptors_;
    mutable ThreadsafeStringMap<AutoTextureAtlasDescriptor> auto_atlas_tile_descriptors_;
    mutable ThreadsafeStringMap<CubemapDescriptor> cubemap_descriptors_;
    mutable ThreadsafeMap<std::pair<std::string, float>, LoadedFont> font_textures_;
    ThreadsafeStringMap<FixedArray<double, 4, 4>> vps_;
    ThreadsafeStringMap<float> offsets_;
    ThreadsafeStringMap<float> discreteness_;
    ThreadsafeStringMap<float> scales_;
    ThreadsafeStringMap<WrapMode> texture_wrap_;
    ThreadsafeStringMap<BlendMapTexture> blend_map_textures_;
    mutable ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    std::string name_;
    unsigned int max_anisotropic_filtering_level_;
    BackgroundLoop preloader_background_loop_;
    DeallocationToken deallocation_token_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
