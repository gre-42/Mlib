#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Render/Fullscreen_Callback.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

class Scene;
class SelectedCameras;
class DeleteNodeMutex;

class StandardCameraLogic: public RenderLogic {
public:
    explicit StandardCameraLogic(
        const Scene& scene,
        SelectedCameras& cameras,
        const DeleteNodeMutex& delete_node_mutex);
    ~StandardCameraLogic();

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual float near_plane() const override;
    virtual float far_plane() const override;
    virtual const FixedArray<double, 4, 4>& vp() const override;
    virtual const TransformationMatrix<float, double, 3>& iv() const override;
    virtual DanglingRef<const SceneNode> camera_node() const override;
    virtual bool requires_postprocessing() const override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void reset();
private:
    const Scene& scene_;
    SelectedCameras& cameras_;
    const DeleteNodeMutex& delete_node_mutex_;
    FixedArray<double, 4, 4> vp_;
    TransformationMatrix<float, double, 3> iv_;
    DanglingPtr<const SceneNode> camera_node_;
};

}
