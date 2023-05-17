#pragma once
#include <cstddef>
#include <list>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct RenderConfig;
struct SceneGraphConfig;
struct ExternalRenderPass;
struct BillboardSequence;

class IParticlesInstance {
public:
    virtual ~IParticlesInstance() = default;
    virtual void add_particle(
        const TransformationMatrix<float, double, 3>& transformation_matrix,
        const BillboardSequence& sequence) = 0;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
