#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>

namespace Mlib {

class Player;
class SceneNode;
class RigidBodyVehicle;
class SkillMap;
enum class ExternalsMode: uint32_t;
enum class ControlSource;
enum class ShutdownPhase;
struct InternalsMode;

class SceneVehicle final: public virtual DanglingBaseClass, public virtual DestructionNotifier {
    SceneVehicle(const SceneVehicle&) = delete;
    SceneVehicle& operator = (const SceneVehicle&) = delete;
public:
    using CreateVehicleExternals = std::function<void(
        const Player& player,
        ExternalsMode externals_mode)>;
    using CreateRoleExternals = std::function<void(
        const Player& player,
        const InternalsMode& internals_mode)>;
    SceneVehicle(
        VariableAndHash<std::string> scene_node_name,
        const DanglingBaseClassRef<SceneNode>& scene_node,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb);
    ~SceneVehicle();
    void create_vehicle_externals(
        const Player& player,
        ExternalsMode externals_mode) const;
    void create_vehicle_internals(
        const Player& player,
        const InternalsMode& internals_mode) const;
    void set_create_vehicle_externals(
        const CreateVehicleExternals& create_vehicle_externals);
    void set_create_vehicle_internals(
        const CreateRoleExternals& create_vehicle_internals);
    VariableAndHash<std::string>& scene_node_name();
    const VariableAndHash<std::string>& scene_node_name() const;
    DanglingBaseClassRef<SceneNode> scene_node();
    DanglingBaseClassRef<const SceneNode> scene_node() const;
    ShutdownPhase scene_node_shutdown_phase() const;
    DanglingBaseClassRef<RigidBodyVehicle> rb();
    DanglingBaseClassRef<const RigidBodyVehicle> rb() const;
private:
    DestructionFunctionsRemovalTokens on_scene_node_destroyed_;
    DestructionFunctionsRemovalTokens on_rigid_body_destroyed_;
    VariableAndHash<std::string> scene_node_name_;
    DanglingBaseClassPtr<SceneNode> scene_node_;
    DanglingBaseClassPtr<RigidBodyVehicle> rb_;
    CreateVehicleExternals create_vehicle_externals_;
    CreateRoleExternals create_vehicle_internals_;
};

}
