#pragma once
#include <Mlib/Activator_Function.hpp>
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers_Hash.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Map/Threadsafe_Map.hpp>
#include <Mlib/Map/Threadsafe_String_Unordered_Map.hpp>
#include <Mlib/Map/Threadsafe_Unordered_Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Resource_Managers/Font_Name_And_Height_Hash.hpp>
#include <Mlib/Scene_Pos.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <functional>
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
enum class InterpolationMode;
enum class ColorMode;
struct LoadedFont;
class ITextureHandle;

struct TextureSizeAndMipmaps {
    GLsizei width;
    GLsizei height;
    GLsizei nchannels;
    GLsizei mip_level_count;
};

enum class ResourceOwner {
    CALLER,
    CONTAINER
};

struct TextureHandleAndOwner {
    GLuint handle;
    ResourceOwner owner;
};

struct ManualAtlasTileSource {
    int left;
    int bottom;
    int width;
    int height;
    std::string filename;
};

struct ManualAtlasTileTarget {
    int left;
    int bottom;
    size_t layer;
};

struct ManualAtlasTileDescriptor {
    ManualAtlasTileSource source;
    ManualAtlasTileTarget target;
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
    size_t nlayers;
    InterpolationMode depth_interpolation;
    ColorMode color_mode;
    std::vector<ManualAtlasTileDescriptor> tiles;
};

struct AutoTextureAtlasDescriptor {
    int width;
    int height;
    int mip_level_count;
    ColorMode color_mode;
    unsigned int anisotropic_filtering_level;
    std::vector<std::vector<AutoAtlasTileDescriptor>> tiles;
};

std::ostream& operator << (std::ostream& ostr, const AutoTextureAtlasDescriptor& atad);

struct CubemapDescriptor {
    std::vector<std::string> filenames;
};

enum class DeletionFailureMode {
    IGNORE,
    WARN,
    ABORT
};

enum class CallerType {
    PRELOAD,
    RENDER
};

enum class CopyBehavior {
    RAISE,
    COPY
};

enum class TextureRole {
    TRUSTED,
    COLOR_FROM_DB,
    COLOR,
    SPECULAR,
    NORMAL
};

enum class TextureType {
    TEXTURE_2D,
    TEXTURE_2D_ARRAY,
    TEXTURE_3D,
    TEXTURE_CUBE_MAP
};

struct TextureSize {
    int width;
    int height;
};

std::ostream& operator << (std::ostream& ostr, TextureType texture_type);

class RenderingResources final: public IDdsResources {
    RenderingResources(const RenderingResources&) = delete;
    RenderingResources& operator = (const RenderingResources&) = delete;
public:
    explicit RenderingResources(
        std::string name,
        unsigned int max_anisotropic_filtering_level);
    ~RenderingResources();
    const ColormapWithModifiers& colormap(const ColormapWithModifiers& name) const;
    void preload(const TextureDescriptor& descriptor) const;
    bool texture_is_loaded_and_try_preload(
        const ColormapWithModifiers& color,
        TextureRole role = TextureRole::COLOR);
    GLuint get_texture(
        const ColormapWithModifiers& name,
        TextureRole role = TextureRole::COLOR,
        CallerType caller_type = CallerType::RENDER) const;
    bool contains_texture(const ColormapWithModifiers& name) const;
    TextureType texture_type(const ColormapWithModifiers& name, TextureRole role) const;
    void set_texture(
        const ColormapWithModifiers& name,
        GLuint id,
        ResourceOwner resource_owner,
        const TextureSize* texture_size = nullptr);
    void set_texture(
        const ColormapWithModifiers& name,
        std::unique_ptr<ITextureHandle>&& id,
        const TextureSize* texture_size = nullptr);
    void set_textures_lazy(std::function<void()> func);
    void add_texture_descriptor(const std::string& name, const TextureDescriptor& descriptor);
    TextureDescriptor get_existing_texture_descriptor(const std::string& name) const;
    void add_manual_texture_atlas(const std::string& name, const ManualTextureAtlasDescriptor& texture_atlas_descriptor);
    std::map<std::string, ManualUvTile> generate_manual_texture_atlas(
        const std::string& name,
        const std::vector<std::string>& filenames);
    std::map<std::string, AutoUvTile> generate_auto_texture_atlas(
        const std::string& name,
        const std::vector<std::string>& filenames,
        int mip_level_count,
        unsigned int anisotropic_filtering_level,
        int size = 4096,
        AutoTextureAtlasDescriptor* atlas = nullptr);
    void add_cubemap(const std::string& name, const std::vector<std::string>& filenames);

    std::string get_texture_filename(
        const ColormapWithModifiers& color,
        TextureRole role,
        const std::string& default_filename) const;

    BlendMapTexture get_blend_map_texture(const std::string& name) const;
    void set_blend_map_texture(const std::string& name, const BlendMapTexture& bmt);

    void set_alias(std::string alias, std::string name);
    std::string get_alias(const std::string& alias) const;
    bool contains_alias(const std::string& alias) const;
    const FixedArray<ScenePos, 4, 4>& get_vp(const std::string& name) const;
    void set_vp(const std::string& name, const FixedArray<ScenePos, 4, 4>& vp);
    float get_offset(const std::string& name) const;
    void set_offset(const std::string& name, float value);
    float get_discreteness(const std::string& name) const;
    void set_discreteness(const std::string& name, float value);
    float get_scale(const std::string& name) const;
    void set_scale(const std::string& name, float value);
    WrapMode get_texture_wrap(const std::string& name) const;
    void set_texture_wrap(const std::string& name, WrapMode mode);

    void delete_vp(const std::string& name, DeletionFailureMode deletion_failure_mode);
    void delete_texture(const ColormapWithModifiers& name, DeletionFailureMode deletion_failure_mode);

    ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    const std::string& name() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

    std::vector<StbInfo<uint8_t>> get_texture_array_data(
        const ColormapWithModifiers& color,
        TextureRole role,
        FlipMode flip_mode,
        CopyBehavior copy_behavior = CopyBehavior::RAISE) const;

    StbInfo<uint8_t> get_texture_data(
        const ColormapWithModifiers& color,
        TextureRole role,
        FlipMode flip_mode,
        CopyBehavior copy_behavior = CopyBehavior::RAISE) const;

    const LoadedFont& get_font_texture(const FontNameAndHeight& font_descriptor) const;

    void save_to_file(const std::string& filename, const ColormapWithModifiers& color, TextureRole role) const;
    void save_array_to_file(const std::string& filename_prefix, const ColormapWithModifiers& color, TextureRole role) const;

    virtual void add_texture(
        const std::string& name,
        std::vector<uint8_t>&& data,
        TextureAlreadyExistsBehavior already_exists_behavior) override;

private:
    GLuint get_cubemap_unsafe(const std::string& name) const;
    void preload(const ColormapWithModifiers& color, TextureRole role) const;
    bool texture_is_loaded_unsafe(const ColormapWithModifiers& name) const;
    void deallocate();
    std::pair<GLuint, TextureType> initialize_non_dds_texture(const ColormapWithModifiers& name, TextureRole role, float aniso) const;
    TextureSizeAndMipmaps initialize_dds_texture(const ColormapWithModifiers& name) const;
    void add_auto_texture_atlas(const std::string& name, const AutoTextureAtlasDescriptor& texture_atlas_descriptor);
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable std::list<std::shared_ptr<ActivationState>> set_textures_lazy_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, StbInfo<uint8_t>> preloaded_processed_texture_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, std::vector<StbInfo<uint8_t>>> preloaded_processed_texture_array_data_;
    mutable ThreadsafeStringUnorderedMap<std::vector<uint8_t>> preloaded_raw_texture_data_;
    mutable ThreadsafeStringUnorderedMap<std::vector<uint8_t>> preloaded_texture_dds_data_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureType> texture_types_;
    mutable ThreadsafeUnorderedMap<VariableAndHash<std::string>, TextureDescriptor> texture_descriptors_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureHandleAndOwner> textures_;
    mutable ThreadsafeStringUnorderedMap<TextureSize> texture_sizes_;
    mutable ThreadsafeStringUnorderedMap<ManualTextureAtlasDescriptor> manual_atlas_tile_descriptors_;
    mutable ThreadsafeStringUnorderedMap<AutoTextureAtlasDescriptor> auto_atlas_tile_descriptors_;
    mutable ThreadsafeStringUnorderedMap<CubemapDescriptor> cubemap_descriptors_;
    mutable VerboseUnorderedMap<FontNameAndHeight, LoadedFont> font_textures_;
    ThreadsafeStringUnorderedMap<std::string> aliases_;
    ThreadsafeStringUnorderedMap<FixedArray<ScenePos, 4, 4>> vps_;
    ThreadsafeStringUnorderedMap<float> offsets_;
    ThreadsafeStringUnorderedMap<float> discreteness_;
    ThreadsafeStringUnorderedMap<float> scales_;
    ThreadsafeStringUnorderedMap<WrapMode> texture_wrap_;
    ThreadsafeStringUnorderedMap<BlendMapTexture> blend_map_textures_;
    mutable ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    std::string name_;
    unsigned int max_anisotropic_filtering_level_;
    BackgroundLoop preloader_background_loop_;
    DeallocationToken deallocation_token_;
    std::shared_ptr<int> lifetime_indicator_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
