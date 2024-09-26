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

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual bool render_optional_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup* setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void set_background_color(const FixedArray<float, 3>& color);
private:
    std::unique_ptr<StandardCameraLogic> standard_camera_logic_;
    std::unique_ptr<StandardRenderLogic> standard_render_logic_;
};

}
