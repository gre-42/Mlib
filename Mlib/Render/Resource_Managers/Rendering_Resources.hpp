#pragma once
#include <Mlib/Activator_Function.hpp>
#include <Mlib/Array/Array_Forward.hpp>
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Interfaces/IDds_Resources.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Colormap_With_Modifiers_Hash.hpp>
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
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

template <class TData>
class StbInfo;

namespace Mlib {

enum class FlipMode;
enum class TextureWarnFlags;
struct TextureDescriptor;
struct RenderProgramIdentifier;
struct ColoredRenderProgram;
class RenderingResources;
struct BlendMapTexture;
enum class InterpolationMode;
enum class ColorMode;
struct LoadedFont;
class ITextureHandle;
enum class TextureTarget;

struct TextureSizeAndMipmaps {
    std::shared_ptr<ITextureHandle> handle;
    GLsizei width;
    GLsizei height;
    GLsizei nchannels;
    GLsizei mip_level_count;
    FixedArray<float, 4> border_color;
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
    MipmapMode mipmap_mode;
    FixedArray<WrapMode, 2> wrap_modes;
    InterpolationMode magnifying_interpolation_mode;
    InterpolationMode depth_interpolation;
    unsigned int anisotropic_filtering_level;
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

struct TextureSize {
    int width;
    int height;
};

struct FlippedTextureData {
    std::vector<std::byte> data;
    FlipMode flip_mode;
};

struct InitializedTexture {
    GLuint handle;
    TextureTarget target;
    uint32_t layers;
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
        const VariableAndHash<std::string>& name,
        TextureRole role = TextureRole::COLOR,
        CallerType caller_type = CallerType::RENDER) const;
    std::shared_ptr<ITextureHandle> get_texture(
        const ColormapWithModifiers& name,
        TextureRole role = TextureRole::COLOR,
        CallerType caller_type = CallerType::RENDER) const;
    std::shared_ptr<ITextureHandle> get_texture_lazy(
        const ColormapWithModifiers& name,
        TextureRole role = TextureRole::COLOR) const;
    bool contains_texture(const ColormapWithModifiers& name) const;
    TextureTarget texture_target(const ColormapWithModifiers& name, TextureRole role) const;
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
    void add_texture_descriptor(VariableAndHash<std::string> name, TextureDescriptor descriptor);
    void add_colormap(VariableAndHash<std::string> name, ColormapWithModifiers colormap);
    bool contains_texture_descriptor(const VariableAndHash<std::string>& name) const;
    bool contains_colormap(const VariableAndHash<std::string>& name) const;
    const TextureDescriptor& get_texture_descriptor(const VariableAndHash<std::string>& name) const;
    const ColormapWithModifiers& get_colormap(const VariableAndHash<std::string>& name) const;
    void add_manual_texture_atlas(VariableAndHash<std::string> name, const ManualTextureAtlasDescriptor& texture_atlas_descriptor);
    std::map<ColormapWithModifiers, ManualUvTile> generate_manual_texture_atlas(
        const VariableAndHash<std::string>& name,
        const std::vector<ColormapWithModifiers>& filenames);
    std::unordered_map<VariableAndHash<std::string>, AutoUvTile> generate_auto_texture_atlas(
        const ColormapWithModifiers& name,
        const std::vector<ColormapWithModifiers>& filenames,
        int mip_level_count,
        int size = 4096,
        AutoTextureAtlasDescriptor* atlas = nullptr);
    void add_cubemap(VariableAndHash<std::string> name, const std::vector<VariableAndHash<std::string>>& filenames);

    std::string get_texture_filename(
        const ColormapWithModifiers& color,
        TextureRole role,
        const std::string& default_filename) const;

    BlendMapTexture get_blend_map_texture(const VariableAndHash<std::string>& name) const;
    void set_blend_map_texture(const VariableAndHash<std::string>& name, const BlendMapTexture& bmt);

    void set_suppressed_warnings(VariableAndHash<std::string> name, TextureWarnFlags warn_flags);
    TextureWarnFlags get_suppressed_warnings(const VariableAndHash<std::string>& name) const;
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

    void delete_vp(const VariableAndHash<std::string>& name, DeletionFailureMode deletion_failure_mode);
    void delete_texture(const ColormapWithModifiers& name, DeletionFailureMode deletion_failure_mode);

    ThreadsafeMap<RenderProgramIdentifier, std::unique_ptr<ColoredRenderProgram>>& render_programs();

    const std::string& name() const;
    void print(std::ostream& ostr, size_t indentation = 0) const;

    std::vector<std::shared_ptr<StbInfo<uint8_t>>> get_texture_array_data(
        const ColormapWithModifiers& color,
        TextureRole role,
        FlipMode flip_mode) const;

    std::shared_ptr<StbInfo<uint8_t>> get_texture_data(
        const ColormapWithModifiers& color,
        TextureRole role,
        FlipMode flip_mode) const;

    void add_charset(VariableAndHash<std::string> name, const std::u32string& charset);
    const std::unordered_map<char32_t, uint32_t>& get_charset(const VariableAndHash<std::string>& name) const;
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
    InitializedTexture initialize_non_dds_texture(const ColormapWithModifiers& name, TextureRole role, float aniso) const;
    std::shared_ptr<ITextureHandle> initialize_dds_texture(const ColormapWithModifiers& name, float aniso) const;
    void add_auto_texture_atlas(
        const ColormapWithModifiers& name,
        const AutoTextureAtlasDescriptor& texture_atlas_descriptor);
    template <class TContainer, class... TArgs>
    auto& add(TContainer& container, TArgs&&... args) const;
    template <class TContainer, class... TArgs>
    auto& add_font(TContainer& container, TArgs&&... args) const;
    template <bool textract, class TContainer, class TKey>
    auto get_or_extract(TContainer& container, const TKey& key) const;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    mutable SafeAtomicSharedMutex font_mutex_;
    mutable std::list<std::shared_ptr<ActivationState>> set_textures_lazy_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, std::shared_ptr<StbInfo<uint8_t>>> preloaded_processed_texture_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, std::vector<std::shared_ptr<StbInfo<uint8_t>>>> preloaded_processed_texture_array_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, FlippedTextureData> preloaded_raw_texture_data_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, FlippedTextureData> preloaded_texture_dds_data_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureTarget> texture_targets_;
    mutable ThreadsafeUnorderedMap<VariableAndHash<std::string>, TextureDescriptor> texture_descriptors_;
    mutable VerboseUnorderedMap<ColormapWithModifiers, TextureHandleAndOwner> textures_;
    mutable ThreadsafeStringWithHashUnorderedMap<TextureSize> texture_sizes_;
    mutable ThreadsafeStringWithHashUnorderedMap<ManualTextureAtlasDescriptor> manual_atlas_tile_descriptors_;
    mutable ThreadsafeUnorderedMap<ColormapWithModifiers, AutoTextureAtlasDescriptor> auto_atlas_tile_descriptors_;
    mutable ThreadsafeStringWithHashUnorderedMap<CubemapDescriptor> cubemap_descriptors_;
    mutable ThreadsafeStringWithHashUnorderedMap<ColormapWithModifiers> colormap_descriptors_;
    mutable ThreadsafeStringWithHashUnorderedMap<std::unordered_map<char32_t, uint32_t>> charsets_;
    mutable VerboseUnorderedMap<FontNameAndHeight, LoadedFont> font_textures_;
    ThreadsafeStringWithHashUnorderedMap<TextureWarnFlags> suppressed_warnings_;
    ThreadsafeStringWithHashUnorderedMap<VariableAndHash<std::string>> aliases_;
    ThreadsafeStringWithHashUnorderedMap<FixedArray<ScenePos, 4, 4>> vps_;
    ThreadsafeStringWithHashUnorderedMap<float> offsets_;
    ThreadsafeStringWithHashUnorderedMap<float> discreteness_;
    ThreadsafeStringWithHashUnorderedMap<float> scales_;
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
