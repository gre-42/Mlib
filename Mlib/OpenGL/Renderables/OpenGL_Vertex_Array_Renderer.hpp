#pragma once
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Render/IGpu_Vertex_Array_Renderer.hpp>
#include <list>
#include <memory>

namespace Mlib {

class IGpuVertexArray;
struct SceneGraphConfig;
template <class TDir, class TPos>
class OffsetAndQuaternion;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct DynamicStyle;
struct Light;
struct Skidmark;
struct RenderConfig;
struct RenderPass;
struct AnimationState;
struct ColorStyle;
class RenderingResources;
struct RenderProgramIdentifier;
struct AnimatedColoredVertexArrays;
struct ColoredRenderProgram;
struct BlendMapTextureAndId;

class OpenGLVertexArrayRenderer: public IGpuVertexArrayRenderer {
public:
    OpenGLVertexArrayRenderer(
        RenderingResources& primary_rendering_resources,
        RenderingResources& secondary_rendering_resources);
    virtual void render(
        const std::shared_ptr<IGpuVertexArray>& cva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& acvas,
        const UUVector<OffsetAndQuaternion<float, float>>& absolute_bone_transformations,
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<SceneDir, ScenePos, 3>& m,
        const TransformationMatrix<SceneDir, ScenePos, 3>& iv,
        const DynamicStyle* dynamic_style,
        const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<SceneDir, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const override;
private:
    const ColoredRenderProgram& get_render_program(
        const RenderProgramIdentifier& id,
        const IGpuVertexArray& gva,
        const std::shared_ptr<AnimatedColoredVertexArrays>& acvas,
        const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& filtered_lights,
        const std::vector<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& filtered_skidmarks,
        const std::vector<size_t>& lightmap_indices,
        const std::vector<size_t>& light_noshadow_indices,
        const std::vector<size_t>& light_shadow_indices,
        const std::vector<size_t>& black_shadow_indices,
        const std::vector<BlendMapTextureAndId>& textures_color,
        const std::vector<BlendMapTextureAndId>& textures_alpha) const;
    RenderingResources& primary_rendering_resources_;
    RenderingResources& secondary_rendering_resources_;
};

}