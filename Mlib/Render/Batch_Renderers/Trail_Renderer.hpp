#pragma once
#include <Mlib/Map/Threadsafe_Default_Map.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Renderer.hpp>
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <memory>

namespace Mlib {

class TrailResources;
class TrailsInstance;

class TrailRenderer : public ITrailRenderer {
public:
    explicit TrailRenderer(TrailResources& resources);
    virtual ~TrailRenderer() override;

    // ITrailRenderer
    virtual ITrailStorage& get_storage(const std::string& name) override;
    virtual void preload(const std::string& name) override;
    virtual void move(float dt, std::chrono::steady_clock::time_point time) override;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const override;

private:
    mutable SafeRecursiveSharedMutex mutex_;
    TrailResources& resources_;
    ThreadsafeDefaultMap<std::shared_ptr<TrailsInstance>> instances_;
    ThreadsafeDefaultMap<std::unique_ptr<ITrailStorage>> instantiators_;
};

}
