#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Destruction_Notifier.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <cstddef>
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

class SceneVehicle final: public virtual DanglingBaseClass, public virtual DestructionNotifier {
    SceneVehicle(const SceneVehicle&) = delete;
    SceneVehicle& operator = (const SceneVehicle&) = delete;
public:
    using CreateVehicleExternals = std::function<void(
        uint32_t user_id,
        const std::string& user_name,
        const std::string& player_name,
        ExternalsMode externals_mode,
        const std::string& behavior)>;
    using CreateRoleExternals = std::function<void(
        uint32_t user_id,
        const std::string& user_name,
        const std::string& player_name,
        ExternalsMode externals_mode,
        const SkillMap& skills,
        const std::string& behavior,
        const InternalsMode& internals_mode)>;
    SceneVehicle(
        DeleteNodeMutex& delete_node_mutex,
        VariableAndHash<std::string> scene_node_name,
        const DanglingRef<SceneNode>& scene_node,
        const DanglingBaseClassRef<RigidBodyVehicle>& rb);
    ~SceneVehicle();
    void create_vehicle_externals(
        uint32_t user_id,
        const std::string& user_name,
        const std::string& player_name,
        ExternalsMode externals_mode,
        const std::string& behavior) const;
    void create_vehicle_internals(
        uint32_t user_id,
        const std::string& user_name,
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
    DanglingRef<SceneNode> scene_node();
    DanglingRef<const SceneNode> scene_node() const;
    DanglingBaseClassRef<RigidBodyVehicle> rb();
    DanglingBaseClassRef<const RigidBodyVehicle> rb() const;
private:
    DeleteNodeMutex& delete_node_mutex_;
    DestructionFunctionsRemovalTokens on_scene_node_destroyed_;
    DestructionFunctionsRemovalTokens on_rigid_body_destroyed_;
    VariableAndHash<std::string> scene_node_name_;
    DanglingPtr<SceneNode> scene_node_;
    DanglingBaseClassPtr<RigidBodyVehicle> rb_;
    CreateVehicleExternals create_vehicle_externals_;
    CreateRoleExternals create_vehicle_internals_;
};

}
