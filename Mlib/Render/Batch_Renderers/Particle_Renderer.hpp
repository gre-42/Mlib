#pragma once
#include <Mlib/Map/Threadsafe_Default_Map.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class ParticleResources;
class ParticlesInstance;

class ParticleRenderer : public IParticleRenderer {
public:
    explicit ParticleRenderer(ParticleResources& resources);
    virtual ~ParticleRenderer() override;

    // IParticleRenderer
    virtual IParticleCreator& get_instantiator(const VariableAndHash<std::string>& name) override;
    virtual void preload(const std::string& name) override;
    virtual void move(float dt, const StaticWorld& world) override;
    virtual void render(
        ParticleSubstrate substrate,
        const FixedArray<ScenePos, 4, 4>& vp,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;

private:
    ParticleResources& resources_;
    ThreadsafeDefaultMap<std::shared_ptr<ParticlesInstance>> instances_;
    ThreadsafeDefaultMap<std::unique_ptr<IParticleCreator>> instantiators_;
};

}
