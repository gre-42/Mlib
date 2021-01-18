#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Idle_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Absolute_Movable_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Camera_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Gun_Key_Binding.hpp>
#include <Mlib/Render/Key_Bindings/Relative_Movable_Key_Binding.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>

namespace Mlib {

struct SelectedCameras;
class ButtonPress;
class Scene;

class KeyBindings: public DestructionObserver, public ExternalForceProvider {
public:
    KeyBindings(
        ButtonPress& button_press,
        bool print_gamepad_buttons,
        SelectedCameras& selected_cameras,
        const std::list<Focus>& focus,
        const Scene& scene);
    ~KeyBindings();

    virtual void notify_destroyed(void* destroyed_object) override;

    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBody>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;

    void add_camera_key_binding(const CameraKeyBinding& b);
    void add_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& b);
    void add_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& b);
    void add_relative_movable_key_binding(const RelativeMovableKeyBinding& b);
    void add_gun_key_binding(const GunKeyBinding& b);
private:
    std::list<CameraKeyBinding> camera_key_bindings_;
    std::list<AbsoluteMovableIdleBinding> absolute_movable_idle_bindings_;
    std::list<AbsoluteMovableKeyBinding> absolute_movable_key_bindings_;
    std::list<RelativeMovableKeyBinding> relative_movable_key_bindings_;
    std::list<GunKeyBinding> gun_key_bindings_;

    ButtonPress& button_press_;
    bool print_gamepad_buttons_;
    const Scene& scene_;
    SelectedCameras& selected_cameras_;
    const std::list<Focus>& focus_;
};

}
