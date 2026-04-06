#pragma once
#include <Mlib/Initialization/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
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
struct AnimatedColoredVertexArrays;

class IGpuVertexArrayRenderer {
public:
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
        const ColorStyle* color_style) const = 0;
private:
};

}