#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
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

class TrailsInstance {
    TrailsInstance(const TrailsInstance&) = delete;
    TrailsInstance& operator=(const TrailsInstance&) = delete;

public:
    explicit TrailsInstance(
        const std::string& texture,
        size_t max_num_segments,
        const RenderableResourceFilter& filter);
    ~TrailsInstance();

    void add_triangle(
        const FixedArray<ColoredVertex<double>, 3>& triangle,
        const FixedArray<float, 3>& time,
        const TrailSequence& trail_sequence);
    void move(float dt);
    double time() const;
    void preload() const;
    void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const;

private:
    double time_;
    FixedArray<double, 3> offset_;
    std::shared_ptr<AnimatedTextureLayer> dynamic_vertex_buffers_;
    std::shared_ptr<ColoredVertexArrayResource> cvar_;
    std::unique_ptr<RenderableColoredVertexArray> rcva_;
    RenderableResourceFilter filter_;
};

}
