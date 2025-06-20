#pragma once
#include <Mlib/Map/Threadsafe_Default_Map.hpp>
#include <Mlib/Scene_Graph/Interfaces/IParticle_Renderer.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class ParticleResources;
class ParticlesInstance;
enum class ParticleType;

class ParticleRenderer : public IParticleRenderer {
public:
    explicit ParticleRenderer(ParticleResources& resources, ParticleType particle_type);
    virtual ~ParticleRenderer() override;

    // IParticleRenderer
    virtual IParticleCreator& get_instantiator(const VariableAndHash<std::string>& name) override;
    virtual void preload(const VariableAndHash<std::string>& name) override;
    
    // IAdvanceTime
    virtual void advance_time(float dt, const StaticWorld& world) override;

    // Renderable
    virtual PhysicsMaterial physics_attributes() const override;
    virtual RenderingStrategies rendering_strategies() const override;
    virtual bool requires_render_pass(ExternalRenderPassType render_pass) const override;
    virtual BlendingPassType required_blending_passes(ExternalRenderPassType render_pass) const override;
    virtual ScenePos max_center_distance2(BillboardId billboard_id) const override;
    virtual void render(
        const FixedArray<ScenePos, 4, 4>& mvp,
        const TransformationMatrix<float, ScenePos, 3>& m,
        const TransformationMatrix<float, ScenePos, 3>& iv,
        const DynamicStyle* dynamic_style,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Light>>>& lights,
        const std::list<std::pair<TransformationMatrix<float, ScenePos, 3>, std::shared_ptr<Skidmark>>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const RenderPass& render_pass,
        const AnimationState* animation_state,
        const ColorStyle* color_style) const override;

private:
    ParticleType particle_type_;
    ParticleResources& resources_;
    ThreadsafeDefaultMap<std::shared_ptr<ParticlesInstance>> instances_;
    ThreadsafeDefaultMap<std::unique_ptr<IParticleCreator>> instantiators_;
};

}
