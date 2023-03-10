#pragma once
#include <Mlib/Map.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
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
struct AvatarControllerIdleBinding;
struct AvatarControllerKeyBinding;
struct WeaponCycleKeyBinding;
struct GunKeyBinding;
struct PlayerKeyBinding;
class SelectedCameras;
class ButtonPress;
class GamepadAnalogAxesPosition;
class CursorStates;
class Scene;
class Focuses;
class Players;
struct BaseKeyCombination;
struct BaseGamepadAnalogAxesBinding;
struct KeyConfiguration;

class KeyBindings: public DestructionObserver, public ExternalForceProvider, public RenderLogic {
public:
    KeyBindings(
        ButtonPress& button_press,
        bool print_gamepad_buttons,
        GamepadAnalogAxesPosition& gamepad_analog_axes_position,
        CursorStates& cursor_states,
        SelectedCameras& selected_cameras,
        const Focuses& focuses,
        Players& players);
    ~KeyBindings();

    virtual void notify_destroyed(const Object& destroyed_object) override;

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

    void load_key_configurations(
        const std::string& filename,
        const std::string& fallback_filename);

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
    float get_alpha(
        const BaseKeyCombination& base_combo,
        const BaseGamepadAnalogAxesBinding& base_gamepad_analog_axis,
        const std::string& role);

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

    Map<std::string, KeyConfiguration> key_configurations_;

    ButtonPress& button_press_;
    bool print_gamepad_buttons_;
    GamepadAnalogAxesPosition& gamepad_analog_axes_position_;
    CursorStates& cursor_states_;
    SelectedCameras& selected_cameras_;
    const Focuses& focuses_;
    Players& players_;
};

}
