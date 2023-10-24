#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Resources/Renderable_Resource_Filter.hpp>
#include <list>
#include <memory>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
template <class TPos>
class ColoredVertexArray;
class ColoredVertexArrayResource;
class RenderableColoredVertexArray;
class DynamicInstanceBuffers;
enum class TransformationMode;
struct BillboardSequence;
struct Light;
struct SceneGraphConfig;
struct RenderConfig;
struct ExternalRenderPass;

class ParticlesInstance {
    ParticlesInstance(const ParticlesInstance &) = delete;
    ParticlesInstance &operator=(const ParticlesInstance &) = delete;

public:
    explicit ParticlesInstance(
        const std::shared_ptr<ColoredVertexArray<float>> &triangles,
        size_t max_num_instances,
        const RenderableResourceFilter &filter);
    ~ParticlesInstance();

    void add_particle(const TransformationMatrix<float, double, 3> &transformation_matrix,
                      const BillboardSequence &sequence);

    void move(float dt);

    void preload() const;

    void render(const FixedArray<double, 4, 4> &vp,
                const TransformationMatrix<float, double, 3> &iv,
                const std::list<std::pair<TransformationMatrix<float, double, 3>, Light *>> &lights,
                const SceneGraphConfig &scene_graph_config,
                const RenderConfig &render_config,
                const ExternalRenderPass &external_render_pass) const;

private:
    FixedArray<double, 3> offset_;
    std::shared_ptr<DynamicInstanceBuffers> dynamic_instance_buffers_;
    std::shared_ptr<ColoredVertexArrayResource> cvar_;
    std::unique_ptr<RenderableColoredVertexArray> rcva_;
    RenderableResourceFilter filter_;
};

}
