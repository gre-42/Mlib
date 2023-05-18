#pragma once
#include <Mlib/Map/Threadsafe_Default_Map.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class ParticleResources;
class ParticlesInstance;

class ParticleRenderer: public IParticleRenderer {
public:
    explicit ParticleRenderer(ParticleResources& resources);
    virtual ~ParticleRenderer() override;

    // IParticleRenderer
    virtual IParticleInstantiator& get_instantiator(const std::string& name) override;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;
private:
    mutable RecursiveSharedMutex mutex_;
    ThreadsafeDefaultMap<std::shared_ptr<ParticlesInstance>> instances_;
    ThreadsafeDefaultMap<std::unique_ptr<IParticleInstantiator>> instantiators_;
};

}
