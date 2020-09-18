#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;
struct SelectedCameras;

class StandardCameraLogic: public RenderLogic {
public:
    explicit StandardCameraLogic(const Scene& scene, SelectedCameras& cameras);

    virtual void render(
        int width,
        int height,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<float, 4, 4>& vp() const override;
    virtual const FixedArray<float, 4, 4>& iv() const override;
    virtual bool requires_postprocessing() const override;
private:
    const Scene& scene_;
    SelectedCameras& cameras_;
    FixedArray<float, 4, 4> vp_;
    FixedArray<float, 4, 4> iv_;
};

}
