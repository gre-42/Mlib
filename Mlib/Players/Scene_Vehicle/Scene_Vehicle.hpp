#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <functional>
#include <string>

namespace Mlib {

class DeleteNodeMutex;
class SceneNode;
class RigidBodyVehicle;
class SkillMap;
enum class ExternalsMode;
enum class ControlSource;
struct InternalsMode;

class SceneVehicle {
    SceneVehicle(const SceneVehicle&) = delete;
    SceneVehicle& operator = (const SceneVehicle&) = delete;
public:
    using CreateVehicleExternals = std::function<void(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const std::string& behavior)>;
    using CreateRoleExternals = std::function<void(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const SkillMap& skills,
        const std::string& behavior,
        const InternalsMode& internals_mode)>;
    SceneVehicle(
        DeleteNodeMutex& delete_node_mutex,
        VariableAndHash<std::string> scene_node_name,
        const DanglingRef<SceneNode>& scene_node,
        RigidBodyVehicle& rb);
    ~SceneVehicle();
    void create_vehicle_externals(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const std::string& behavior) const;
    void create_vehicle_internals(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const SkillMap& skills,
        const std::string& behavior,
        const InternalsMode& internals_mode) const;
    void set_create_vehicle_externals(
        const CreateVehicleExternals& create_vehicle_externals);
    void set_create_vehicle_internals(
        const CreateRoleExternals& create_vehicle_internals);
    VariableAndHash<std::string>& scene_node_name();
    const VariableAndHash<std::string>& scene_node_name() const;
    const DanglingRef<SceneNode>& scene_node();
    const DanglingRef<const SceneNode>& scene_node() const;
    RigidBodyVehicle& rb();
    const RigidBodyVehicle& rb() const;
    DestructionObservers<const SceneVehicle&> destruction_observers;
private:
    DeleteNodeMutex& delete_node_mutex_;
    VariableAndHash<std::string> scene_node_name_;
    DanglingRef<SceneNode> scene_node_;
    RigidBodyVehicle& rb_;
    CreateVehicleExternals create_vehicle_externals_;
    CreateRoleExternals create_vehicle_internals_;
};

}
