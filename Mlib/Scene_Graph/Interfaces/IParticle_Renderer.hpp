#pragma once
#include <cstddef>
#include <list>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct RenderConfig;
struct SceneGraphConfig;
struct ExternalRenderPass;
class IParticleInstantiator;

class IParticleRenderer {
public:
    virtual ~IParticleRenderer() = default;
    virtual IParticleInstantiator& get_instantiator(const std::string& resource_name) = 0;
    virtual void move(float dt) = 0;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
