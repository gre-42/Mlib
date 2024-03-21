#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/External_Force_Provider.hpp>
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

struct CameraKeyBinding;
struct AbsoluteMovableIdleBinding;
struct AbsoluteMovableKeyBinding;
struct RelativeMovableKeyBinding;
struct CarControllerIdleBinding;
struct CarControllerKeyBinding;
struct PlaneControllerIdleBinding;
struct PlaneControllerKeyBinding;
struct PrintNodeInfoKeyBinding;
struct AvatarControllerIdleBinding;
struct AvatarControllerKeyBinding;
struct WeaponCycleKeyBinding;
struct GunKeyBinding;
struct PlayerKeyBinding;
class SelectedCameras;
class GamepadAnalogAxesPosition;
class Scene;
class Focuses;
class Players;
struct BaseKeyCombination;
struct BaseGamepadAnalogAxesBinding;
class ButtonPress;
class CursorMovement;
class ScrollWheelMovement;

class KeyBindings: public ExternalForceProvider, public RenderLogic {
public:
    KeyBindings(
        SelectedCameras& selected_cameras,
        const Focuses& focuses,
        Players& players);
    ~KeyBindings();

    // ExternalForceProvider
    virtual void increment_external_forces(const std::list<RigidBodyVehicle*>& olist, bool burn_in, const PhysicsEngineConfig& cfg) override;

    // RenderLogic
    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    void add_camera_key_binding(const CameraKeyBinding& b);
    const AbsoluteMovableIdleBinding& add_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& b);
    const AbsoluteMovableKeyBinding& add_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& b);
    const RelativeMovableKeyBinding& add_relative_movable_key_binding(const RelativeMovableKeyBinding& b);
    const CarControllerIdleBinding& add_car_controller_idle_binding(const CarControllerIdleBinding& b);
    const CarControllerKeyBinding& add_car_controller_key_binding(const CarControllerKeyBinding& b);
    const PlaneControllerIdleBinding& add_plane_controller_idle_binding(const PlaneControllerIdleBinding& b);
    const PlaneControllerKeyBinding& add_plane_controller_key_binding(const PlaneControllerKeyBinding& b);
    const AvatarControllerIdleBinding& add_avatar_controller_idle_binding(const AvatarControllerIdleBinding& b);
    const AvatarControllerKeyBinding& add_avatar_controller_key_binding(const AvatarControllerKeyBinding& b);
    const WeaponCycleKeyBinding& add_weapon_inventory_key_binding(const WeaponCycleKeyBinding& b);
    const GunKeyBinding& add_gun_key_binding(const GunKeyBinding& b);
    const PlayerKeyBinding& add_player_key_binding(const PlayerKeyBinding& b);
    const PrintNodeInfoKeyBinding& add_print_node_info_key_binding(const PrintNodeInfoKeyBinding& b);

    void delete_absolute_movable_idle_binding(const AbsoluteMovableIdleBinding& deleted_key_binding);
    void delete_absolute_movable_key_binding(const AbsoluteMovableKeyBinding& deleted_key_binding);
    void delete_relative_movable_key_binding(const RelativeMovableKeyBinding& deleted_key_binding);
    void delete_car_controller_idle_binding(const CarControllerIdleBinding& deleted_key_binding);
    void delete_car_controller_key_binding(const CarControllerKeyBinding& deleted_key_binding);
    void delete_plane_controller_idle_binding(const PlaneControllerIdleBinding& deleted_key_binding);
    void delete_plane_controller_key_binding(const PlaneControllerKeyBinding& deleted_key_binding);
    void delete_avatar_controller_idle_binding(const AvatarControllerIdleBinding& deleted_key_binding);
    void delete_avatar_controller_key_binding(const AvatarControllerKeyBinding& deleted_key_binding);
    void delete_weapon_cycle_key_binding(const WeaponCycleKeyBinding& deleted_key_binding);
    void delete_gun_key_binding(const GunKeyBinding& deleted_key_binding);
    void delete_player_key_binding(const PlayerKeyBinding& deleted_key_binding);
    void delete_print_node_info_key_binding(const PrintNodeInfoKeyBinding& deleted_key_binding);

    DestructionFunctions on_destroy;
private:
    float get_alpha(
        ButtonPress& button_press,
        CursorMovement* cursor_movement,
        ScrollWheelMovement* scroll_wheel_movement,
        GamepadAnalogAxesPosition* gamepad_analog_axes_position,
        float press_factor,
        float repeat_factor,
        const PhysicsEngineConfig& cfg);

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
    std::list<PrintNodeInfoKeyBinding> print_node_info_key_bindings_;

    SelectedCameras& selected_cameras_;
    const Focuses& focuses_;
    Players& players_;
};

}
