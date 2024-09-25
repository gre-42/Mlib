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

    virtual void init(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void reset() override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<ScenePos, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, ScenePos, 3>& iv() const override;
    virtual DanglingPtr<const SceneNode> camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    const Scene& scene_;
    const SelectedCameras& cameras_;
    FixedArray<ScenePos, 4, 4> vp_;
    TransformationMatrix<float, ScenePos, 3> iv_;
    std::unique_ptr<Camera> camera_;
    DanglingPtr<const SceneNode> camera_node_;
};

}
