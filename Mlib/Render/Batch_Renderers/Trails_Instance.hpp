#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
struct ColoredVertex;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;
class AnimatedTextureLayer;
struct Light;
struct Skidmark;
struct SceneGraphConfig;
struct RenderConfig;
struct ExternalRenderPass;
struct TrailSequence;
struct Shading;
struct StaticWorld;
template <class T>
class VariableAndHash;

class TrailsInstance {
    TrailsInstance(const TrailsInstance&) = delete;
    TrailsInstance& operator=(const TrailsInstance&) = delete;

public:
    explicit TrailsInstance(
        const VariableAndHash<std::string>& texture,
        const Shading& shading,
        const std::vector<float>& continuous_layer_x,
        const std::vector<float>& continuous_layer_y,
        size_t max_num_segments,
        const RenderableResourceFilter& filter);
    ~TrailsInstance();

    void add_triangle(
        const FixedArray<ColoredVertex<ScenePos>, 3>& triangle,
        const FixedArray<float, 3>& time,
        const TrailSequence& trail_sequence);
    void move(float dt, const StaticWorld& world);
    std::chrono::steady_clock::time_point time() const;
    void preload() const;
    void render(
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const;

private:
    FixedArray<ScenePos, 3> offset_;
    std::shared_ptr<AnimatedTextureLayer> dynamic_vertex_buffers_;
    std::shared_ptr<ColoredVertexArrayResource> cvar_;
    std::unique_ptr<RenderableColoredVertexArray> rcva_;
    RenderableResourceFilter filter_;
};

}
