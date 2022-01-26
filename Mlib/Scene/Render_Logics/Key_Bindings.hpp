#pragma once
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>

namespace Mlib {

struct CameraKeyBinding;
struct AbsoluteMovableIdleBinding;
struct AbsoluteMovableKeyBinding;
struct RelativeMovableKeyBinding;
struct CarControllerIdleBinding;
struct CarControllerKeyBinding;
struct AvatarControllerIdleBinding;
struct AvatarControllerKeyBinding;
struct WeaponInventoryKeyBinding;
struct GunKeyBinding;
struct PlayerKeyBinding;
class SelectedCameras;
class ButtonPress;
class Scene;
class Focuses;

class KeyBindings: public DestructionObserver, public ExternalForceProvider {
public:
    KeyBindings(
        ButtonPress& button_press,
        bool print_gamepad_buttons,
        SelectedCameras& selected_cameras,
        const Focuses& focuses);
    ~KeyBindings();

    virtual void notify_destroyed(void* destroyed_object) override;

    virtual void increment_external_forces(const std::list<std::shared_ptr<RigidBodyVehicle>>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;

    void add_camera_key_binding(const CameraKeyBinding& b);
    void add_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& b);
    void add_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& b);
    void add_relative_movable_key_binding(const RelativeMovableKeyBinding& b);
    void add_car_controller_idle_binding(const CarControllerIdleBinding& b);
    void add_car_controller_key_binding(const CarControllerKeyBinding& b);
    void add_avatar_controller_idle_binding(const AvatarControllerIdleBinding& b);
    void add_avatar_controller_key_binding(const AvatarControllerKeyBinding& b);
    void add_weapon_inventory_key_binding(const WeaponInventoryKeyBinding& b);
    void add_gun_key_binding(const GunKeyBinding& b);
    void add_player_key_binding(const PlayerKeyBinding& b);
private:
    std::list<CameraKeyBinding> camera_key_bindings_;
    std::list<AbsoluteMovableIdleBinding> absolute_movable_idle_bindings_;
    std::list<AbsoluteMovableKeyBinding> absolute_movable_key_bindings_;
    std::list<RelativeMovableKeyBinding> relative_movable_key_bindings_;
    std::list<CarControllerIdleBinding> car_controller_idle_bindings_;
    std::list<CarControllerKeyBinding> car_controller_key_bindings_;
    std::list<AvatarControllerIdleBinding> avatar_controller_idle_bindings_;
    std::list<AvatarControllerKeyBinding> avatar_controller_key_bindings_;
    std::list<WeaponInventoryKeyBinding> weapon_inventory_key_bindings_;
    std::list<GunKeyBinding> gun_key_bindings_;
    std::list<PlayerKeyBinding> player_key_bindings_;

    ButtonPress& button_press_;
    bool print_gamepad_buttons_;
    SelectedCameras& selected_cameras_;
    const Focuses& focuses_;
};

}
