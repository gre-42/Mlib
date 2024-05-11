#pragma once
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <functional>
#include <string>

namespace Mlib {

class DeleteNodeMutex;
class SceneNode;
class RigidBodyVehicle;
class SkillMap;
enum class ExternalsMode;
enum class ControlSource;

class SceneVehicle {
    SceneVehicle(const SceneVehicle&) = delete;
    SceneVehicle& operator = (const SceneVehicle&) = delete;
public:
    using CreateExternals = std::function<void(const std::string&, ExternalsMode, const SkillMap&, const std::string&)>;
    SceneVehicle(
        DeleteNodeMutex& delete_node_mutex,
        std::string scene_node_name,
        DanglingRef<SceneNode> scene_node,
        RigidBodyVehicle& rb);
    ~SceneVehicle();
    void create_externals(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const SkillMap& skills,
        const std::string& behavior) const;
    void set_create_externals(const CreateExternals& create_externals);
    std::string& scene_node_name();
    const std::string& scene_node_name() const;
    DanglingRef<SceneNode> scene_node();
    DanglingRef<const SceneNode> scene_node() const;
    RigidBodyVehicle& rb();
    const RigidBodyVehicle& rb() const;
    DestructionObservers<const SceneVehicle&> destruction_observers;
private:
    DeleteNodeMutex& delete_node_mutex_;
    std::string scene_node_name_;
    DanglingRef<SceneNode> scene_node_;
    RigidBodyVehicle& rb_;
    CreateExternals create_externals_;
};

}
