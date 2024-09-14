#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <memory>

namespace Mlib {

class Scene;
enum class ClearMode;
class StandardCameraLogic;
class StandardRenderLogic;
class SelectedCameras;

class CameraStreamLogic: public RenderLogic {
public:
    CameraStreamLogic(
        const Scene& scene,
        const SelectedCameras& selected_cameras,
        const FixedArray<float, 3>& background_color,
        ClearMode clear_mode);
    ~CameraStreamLogic();

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

    void set_background_color(const FixedArray<float, 3>& color);
private:
    std::unique_ptr<StandardCameraLogic> standard_camera_logic_;
    std::unique_ptr<StandardRenderLogic> standard_render_logic_;
};

}
