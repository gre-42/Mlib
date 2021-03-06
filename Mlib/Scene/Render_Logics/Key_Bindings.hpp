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
struct PlaneControllerIdleBinding;
struct PlaneControllerKeyBinding;
struct AvatarControllerIdleBinding;
struct AvatarControllerKeyBinding;
struct WeaponCycleKeyBinding;
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
    const CarControllerIdleBinding& add_car_controller_idle_binding(const CarControllerIdleBinding& b);
    const CarControllerKeyBinding& add_car_controller_key_binding(const CarControllerKeyBinding& b);
    const PlaneControllerIdleBinding& add_plane_controller_idle_binding(const PlaneControllerIdleBinding& b);
    const PlaneControllerKeyBinding& add_plane_controller_key_binding(const PlaneControllerKeyBinding& b);
    void add_avatar_controller_idle_binding(const AvatarControllerIdleBinding& b);
    void add_avatar_controller_key_binding(const AvatarControllerKeyBinding& b);
    void add_weapon_inventory_key_binding(const WeaponCycleKeyBinding& b);
    const GunKeyBinding& add_gun_key_binding(const GunKeyBinding& b);
    const PlayerKeyBinding& add_player_key_binding(const PlayerKeyBinding& b);

    void delete_car_controller_idle_binding(const CarControllerIdleBinding& deleted_key_binding);
    void delete_car_controller_key_binding(const CarControllerKeyBinding& deleted_key_binding);
    void delete_plane_controller_idle_binding(const PlaneControllerIdleBinding& deleted_key_binding);
    void delete_plane_controller_key_binding(const PlaneControllerKeyBinding& deleted_key_binding);
    void delete_gun_key_binding(const GunKeyBinding& deleted_key_binding);
    void delete_player_key_binding(const PlayerKeyBinding& deleted_key_binding);
private:
    std::list<CameraKeyBinding> camera_key_bindings_;
    std::list<AbsoluteMovableIdleBinding> absolute_movable_idle_bindings_;
    std::list<AbsoluteMovableKeyBinding> absolute_movable_key_bindings_;
    std::list<RelativeMovableKeyBinding> relative_movable_key_bindings_;
    std::list<CarControllerIdleBinding> car_controller_idle_bindings_;
    std::list<CarControllerKeyBinding> car_controller_key_bindings_;
    std::list<PlaneControllerIdleBinding> plane_controller_idle_bindings_;
    std::list<PlaneControllerKeyBinding> plane_controller_key_bindings_;
    std::list<AvatarControllerIdleBinding> avatar_controller_idle_bindings_;
    std::list<AvatarControllerKeyBinding> avatar_controller_key_bindings_;
    std::list<WeaponCycleKeyBinding> weapon_cycle_key_bindings_;
    std::list<GunKeyBinding> gun_key_bindings_;
    std::list<PlayerKeyBinding> player_key_bindings_;

    ButtonPress& button_press_;
    bool print_gamepad_buttons_;
    SelectedCameras& selected_cameras_;
    const Focuses& focuses_;
};

}
