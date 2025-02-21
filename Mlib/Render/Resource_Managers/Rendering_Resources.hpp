#pragma once
#include <Mlib/Activator_Function.hpp>
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers_Hash.hpp>
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Geometry/Texture/Uv_Tile.hpp>
#include <Mlib/Map/Threadsafe_Map.hpp>
#include <Mlib/Map/Threadsafe_String_With_Hash_Unordered_Map.hpp>
#include <Mlib/Map/Threadsafe_Unordered_Map.hpp>
#include <Mlib/Map/Verbose_Unordered_Map.hpp>
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Resource_Managers/Font_Name_And_Height_Hash.hpp>
#include <Mlib/Render/Resource_Managers/Texture_Role.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Background_Loop.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

template <class TData>
struct StbInfo;

namespace Mlib {

enum class FlipMode;
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
    std::shared_ptr<ITextureHandle> handle;
    GLsizei width;
    GLsizei height;
    GLsizei nchannels;
    GLsizei mip_level_count;
    std::shared_ptr<ITextureHandle> flipped_vertically(float aniso) const;
};

struct TextureHandleAndOwner {
    std::shared_ptr<ITextureHandle> handle;
};

struct ManualAtlasTileSource {
    int left;
    int bottom;
    int width;
    int height;
    ColormapWithModifiers name;
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
    ColormapWithModifiers name;
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
    std::vector<std::vector<AutoAtlasTileDescriptor>> tiles;
};

std::ostream& operator << (std::ostream& ostr, const AutoTextureAtlasDescriptor& atad);

struct CubemapDescriptor {
    std::vector<VariableAndHash<std::string>> filenames;
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

struct FlippedTextureData {
    std::vector<std::byte> data;
    FlipMode flip_mode;
};

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
        TextureRole role = TextureRole::COLOR) const;
    std::shared_ptr<ITextureHandle> get_texture(
        const ColormapWithModifiers& name,
        TextureRole role = TextureRole::COLOR,
        CallerType caller_type = CallerType::RENDER) const;
    std::shared_ptr<ITextureHandle> get_texture_lazy(
        const ColormapWithModifiers& name,
        TextureRole role = TextureRole::COLOR) const;
    bool contains_texture(const ColormapWithModifiers& name) const;
    TextureType texture_type(const ColormapWithModifiers& name, TextureRole role) const;
    FixedArray<int, 2> texture_size(const ColormapWithModifiers& name) const;
    void add_texture(
        const ColormapWithModifiers& name,
        std::shared_ptr<ITextureHandle> id,
        const TextureSize* texture_size = nullptr);
    void set_texture(
        const ColormapWithModifiers& name,
        std::shared_ptr<ITextureHandle> id,
        const TextureSize* texture_size = nullptr);
    void set_textures_lazy(std::function<void()> func);
    void add_texture_descriptor(const VariableAndHash<std::string>& name, const TextureDescriptor& descriptor);
    TextureDescriptor get_existing_texture_descriptor(const VariableAndHash<std::string>& name) const;
    void add_manual_texture_atlas(const VariableAndHash<std::string>& name, const ManualTextureAtlasDescriptor& texture_atlas_descriptor);
    std::map<ColormapWithModifiers, ManualUvTile> generate_manual_texture_atlas(
        const VariableAndHash<std::string>& name,
        const std::vector<ColormapWithModifiers>& filenames);
    std::unordered_map<VariableAndHash<std::string>, AutoUvTile> generate_auto_texture_atlas(
        const ColormapWithModifiers& name,
        const std::vector<ColormapWithModifiers>& filenames,
        int mip_level_count,
        int size = 4096,
        AutoTextureAtlasDescriptor* atlas = nullptr);
    void add_cubemap(const VariableAndHash<std::string>& name, const std::vector<VariableAndHash<std::string>>& filenames);

    std::string get_texture_filename(
        const ColormapWithModifiers& color,
        TextureRole role,
        const std::string& default_filename) const;

    BlendMapTexture get_blend_map_texture(const VariableAndHash<std::string>& name) const;
    void set_blend_map_texture(const VariableAndHash<std::string>& name, const BlendMapTexture& bmt);

    void set_alias(VariableAndHash<std::string> alias, VariableAndHash<std::string> name);
    VariableAndHash<std::string> get_alias(const VariableAndHash<std::string>& alias) const;
    bool contains_alias(const VariableAndHash<std::string>& alias) const;
    const FixedArray<ScenePos, 4, 4>& get_vp(const VariableAndHash<std::string>& name) const;
    void set_vp(const VariableAndHash<std::string>& name, const FixedArray<ScenePos, 4, 4>& vp);
    float get_offset(const VariableAndHash<std::string>& name) const;
    void set_offset(const VariableAndHash<std::string>& name, float value);
    float get_discreteness(const VariableAndHash<std::string>& name) const;
    void set_discreteness(const VariableAndHash<std::string>& name, float value);
    float get_scale(const VariableAndHash<std::string>& name) const;
    void set_scale(const VariableAndHash<std::string>& name, float value);
    WrapMode get_texture_wrap(const VariableAndHash<std::string>& name) const;
    void set_texture_wrap(const VariableAndHash<std::string>& name, WrapMode mode);

    void delete_vp(const VariableAndHash<std::string>& name, DeletionFailureMode deletion_failure_mode);
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

    void add_charset(VariableAndHash<std::string> name, const std::wstring& charset);
    const std::unordered_map<wchar_t, uint32_t>& get_charset(const VariableAndHash<std::string>& name) const;
    const LoadedFont& get_font_texture(const FontNameAndHeight& font_descriptor) const;

    void save_to_file(const std::string& filename, const ColormapWithModifiers& color, TextureRole role) const;
    void save_array_to_file(const std::string& filename_prefix, const ColormapWithModifiers& color, TextureRole role) const;

    virtual void add_texture(
        const ColormapWithModifiers& color,
        std::vector<std::byte>&& data,
        FlipMode flip_mode,
        TextureAlreadyExistsBehavior already_exists_behavior) override;

private:
    GLuint get_cubemap_unsafe(const VariableAndHash<std::string>& name) const;
    void preload(const ColormapWithModifiers& color, TextureRole role) const;
    bool texture_is_loaded_unsafe(const ColormapWithModifiers& name) const;
    void deallocate();
    std::pair<GLuint, TextureType> initialize_non_dds_texture(const ColormapWithModifiers& name, TextureRole role, float aniso) const;
    std::shared_ptr<ITextureHandle> initialize_dds_texture(const ColormapWithModifiers& name, float aniso) const;
    void add_auto_texture_atlas(
        const ColormapWithModifiers& name,
        const AutoTextureAtlasDescriptor& texture_atlas_descriptor);
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable SafeAtomicSharedMutex font_mutex_;
    mutable std::list<std::shared_ptr<ActivationState>> set_textures_lazy_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, StbInfo<uint8_t>> preloaded_processed_texture_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, std::vector<StbInfo<uint8_t>>> preloaded_processed_texture_array_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, FlippedTextureData> preloaded_raw_texture_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, FlippedTextureData> preloaded_texture_dds_data_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureType> texture_types_;
    mutable ThreadsafeUnorderedMap<VariableAndHash<std::string>, TextureDescriptor> texture_descriptors_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureHandleAndOwner> textures_;
    mutable ThreadsafeStringWithHashUnorderedMap<TextureSize> texture_sizes_;
    mutable ThreadsafeStringWithHashUnorderedMap<ManualTextureAtlasDescriptor> manual_atlas_tile_descriptors_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, AutoTextureAtlasDescriptor> auto_atlas_tile_descriptors_;
    mutable ThreadsafeStringWithHashUnorderedMap<CubemapDescriptor> cubemap_descriptors_;
    mutable ThreadsafeStringWithHashUnorderedMap<std::unordered_map<wchar_t, uint32_t>> charsets_;
    mutable VerboseUnorderedMap<FontNameAndHeight, LoadedFont> font_textures_;
    ThreadsafeStringWithHashUnorderedMap<VariableAndHash<std::string>> aliases_;
    ThreadsafeStringWithHashUnorderedMap<FixedArray<ScenePos, 4, 4>> vps_;
    ThreadsafeStringWithHashUnorderedMap<float> offsets_;
    ThreadsafeStringWithHashUnorderedMap<float> discreteness_;
    ThreadsafeStringWithHashUnorderedMap<float> scales_;
    ThreadsafeStringWithHashUnorderedMap<WrapMode> texture_wrap_;
    ThreadsafeStringWithHashUnorderedMap<BlendMapTexture> blend_map_textures_;
    mutable ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>> render_programs_;
    std::string name_;
    unsigned int max_anisotropic_filtering_level_;
    mutable BackgroundLoop preloader_background_loop_;
    DeallocationToken deallocation_token_;
    std::shared_ptr<int> lifetime_indicator_;
};

std::ostream& operator << (std::ostream& ostr, const RenderingResources& r);

}
