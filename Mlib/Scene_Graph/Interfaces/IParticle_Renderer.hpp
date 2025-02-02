#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct Skidmark;
struct RenderConfig;
struct SceneGraphConfig;
struct ExternalRenderPass;
class IParticleCreator;
enum class ParticleSubstrate;
struct StaticWorld;

class IParticleRenderer {
public:
    virtual ~IParticleRenderer() = default;
    virtual void preload(const std::string& name) = 0;
    virtual IParticleCreator& get_instantiator(const VariableAndHash<std::string>& name) = 0;
    virtual void move(float dt, const StaticWorld& world) = 0;
    virtual void render(
        ParticleSubstrate substrate,
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
