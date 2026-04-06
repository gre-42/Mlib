#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Cached_Make_Shared.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/OpenGL/Render_Logic.hpp>
#include <Mlib/OpenGL/Render_To_Texture/Lowpass.hpp>
#include <cstdint>
#include <string>

namespace Mlib {

class RenderingResources;
enum class ExternalRenderPassType;
class FrameBuffer;
class SceneNode;
struct Light;
class ArrayInstancesRenderers;
class ArrayInstancesRenderer;
class IGpuObjectFactory;
class IGpuVertexArrayRenderer;

class LightmapLogic: public RenderLogic {
public:
    explicit LightmapLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic,
        ExternalRenderPassType render_pass_type,
        DanglingBaseClassRef<SceneNode> light_node,
        std::shared_ptr<Light> light,
        VariableAndHash<std::string> black_node_name,
        bool with_depth_texture,
        int lightmap_width,
        int lightmap_height,
        const FixedArray<uint32_t, 2>& smooth_niterations);
    virtual ~LightmapLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    DestructionFunctionsRemovalTokens on_child_logic_destroy;
    DestructionFunctionsRemovalTokens on_node_clear;
private:
    RenderingResources& rendering_resources_;
    IGpuObjectFactory& gpu_object_factory_;
    IGpuVertexArrayRenderer& gpu_vertex_array_renderer_;
    RenderLogic& child_logic_;
    std::shared_ptr<FrameBuffer> fbs_[2];
    Lowpass lowpass_;
    ExternalRenderPassType render_pass_type_;
    DanglingBaseClassRef<SceneNode> light_node_;
    const VariableAndHash<std::string> black_node_name_;
    std::shared_ptr<Light> light_;
    bool with_depth_texture_;
    int lightmap_width_;
    int lightmap_height_;
    FixedArray<uint32_t, 2> smooth_niterations_;
    CachedMakeShared<ArrayInstancesRenderers> small_sorted_instances_renderers_;
    CachedMakeShared<ArrayInstancesRenderer> large_instances_renderer_;
};

}
