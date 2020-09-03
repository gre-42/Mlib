#pragma once
#include <Mlib/Physics/External_Force_Provider.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

struct SelectedCameras;
class ButtonPress;
class Scene;

class KeyBindings: public ExternalForceProvider, public RenderLogic {
public:
    KeyBindings(
        ButtonPress& button_press,
        bool print_gamepad_buttons,
        SelectedCameras& selected_cameras,
        const Focus& focus,
        const Scene& scene);

    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in) override;

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
    virtual bool requires_postprocessing() const override;

    std::vector<CameraKeyBinding> camera_key_bindings_;
    std::vector<AbsoluteMovableIdleBinding> absolute_movable_idle_bindings_;
    std::vector<AbsoluteMovableKeyBinding> absolute_movable_key_bindings_;
    std::vector<RelativeMovableKeyBinding> relative_movable_key_bindings_;
    std::vector<GunKeyBinding> gun_key_bindings_;

private:
    ButtonPress& button_press_;
    bool print_gamepad_buttons_;
    const Scene& scene_;
    SelectedCameras& selected_cameras_;
    const Focus& focus_;
};

}
