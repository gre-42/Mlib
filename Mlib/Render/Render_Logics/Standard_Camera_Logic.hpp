#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;
class SelectedCameras;
class Camera;
class DeleteNodeMutex;

class StandardCameraLogic: public RenderLogic {
public:
    explicit StandardCameraLogic(
        const Scene& scene,
        const SelectedCameras& cameras);
    ~StandardCameraLogic();
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_with_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup& setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Scene& scene_;
    const SelectedCameras& cameras_;
};

}
