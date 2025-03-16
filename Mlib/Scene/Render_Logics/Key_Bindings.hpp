#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Physics/Interfaces/IExternal_Force_Provider.hpp>
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

class KeyBindings: public IExternalForceProvider, public RenderLogic {
public:
    KeyBindings(
        SelectedCameras& selected_cameras,
        const Focuses& focuses,
        Players& players);
    ~KeyBindings();

    // IExternalForceProvider
    virtual void increment_external_forces(
        const std::list<RigidBodyVehicle*>& olist,
        bool burn_in,
        const PhysicsEngineConfig& cfg,
        const StaticWorld& world) override;

    // RenderLogic
    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

    CameraKeyBinding& add_camera_key_binding(std::unique_ptr<CameraKeyBinding>&& b);
    AbsoluteMovableIdleBinding& add_absolute_movable_idle_binding(std::unique_ptr<AbsoluteMovableIdleBinding>&& b);
    AbsoluteMovableKeyBinding& add_absolute_movable_key_binding(std::unique_ptr<AbsoluteMovableKeyBinding>&& b);
    RelativeMovableKeyBinding& add_relative_movable_key_binding(std::unique_ptr<RelativeMovableKeyBinding>&& b);
    CarControllerIdleBinding& add_car_controller_idle_binding(std::unique_ptr<CarControllerIdleBinding>&& b);
    CarControllerKeyBinding& add_car_controller_key_binding(std::unique_ptr<CarControllerKeyBinding>&& b);
    PlaneControllerIdleBinding& add_plane_controller_idle_binding(std::unique_ptr<PlaneControllerIdleBinding>&& b);
    PlaneControllerKeyBinding& add_plane_controller_key_binding(std::unique_ptr<PlaneControllerKeyBinding>&& b);
    AvatarControllerIdleBinding& add_avatar_controller_idle_binding(std::unique_ptr<AvatarControllerIdleBinding>&& b);
    AvatarControllerKeyBinding& add_avatar_controller_key_binding(std::unique_ptr<AvatarControllerKeyBinding>&& b);
    WeaponCycleKeyBinding& add_weapon_inventory_key_binding(std::unique_ptr<WeaponCycleKeyBinding>&& b);
    GunKeyBinding& add_gun_key_binding(std::unique_ptr<GunKeyBinding>&& b);
    PlayerKeyBinding& add_player_key_binding(std::unique_ptr<PlayerKeyBinding>&& b);
    PrintNodeInfoKeyBinding& add_print_node_info_key_binding(std::unique_ptr<PrintNodeInfoKeyBinding>&& b);

    // Not yet used.
    // void delete_camera_key_binding(const CameraKeyBinding& deleted_key_binding);
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

private:
    std::list<std::unique_ptr<CameraKeyBinding>> camera_key_bindings_;
    std::list<std::unique_ptr<AbsoluteMovableIdleBinding>> absolute_movable_idle_bindings_;
    std::list<std::unique_ptr<AbsoluteMovableKeyBinding>> absolute_movable_key_bindings_;
    std::list<std::unique_ptr<RelativeMovableKeyBinding>> relative_movable_key_bindings_;
    std::list<std::unique_ptr<CarControllerIdleBinding>> car_controller_idle_bindings_;
    std::list<std::unique_ptr<CarControllerKeyBinding>> car_controller_key_bindings_;
    std::list<std::unique_ptr<PlaneControllerIdleBinding>> plane_controller_idle_bindings_;
    std::list<std::unique_ptr<PlaneControllerKeyBinding>> plane_controller_key_bindings_;
    std::list<std::unique_ptr<AvatarControllerIdleBinding>> avatar_controller_idle_bindings_;
    std::list<std::unique_ptr<AvatarControllerKeyBinding>> avatar_controller_key_bindings_;
    std::list<std::unique_ptr<WeaponCycleKeyBinding>> weapon_cycle_key_bindings_;
    std::list<std::unique_ptr<GunKeyBinding>> gun_key_bindings_;
    std::list<std::unique_ptr<PlayerKeyBinding>> player_key_bindings_;
    std::list<std::unique_ptr<PrintNodeInfoKeyBinding>> print_node_info_key_bindings_;

    SelectedCameras& selected_cameras_;
    const Focuses& focuses_;
    Players& players_;
};

}
