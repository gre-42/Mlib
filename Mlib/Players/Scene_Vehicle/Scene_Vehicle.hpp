#pragma once
#include <Mlib/Memory/Destruction_Observers.hpp>
#include <Mlib/Object.hpp>
#include <functional>
#include <string>
#include <unordered_map>

namespace Mlib {

class DeleteNodeMutex;
class SceneNode;
class RigidBodyVehicle;
struct Skills;
enum class ExternalsMode;
enum class ControlSource;

class SceneVehicle: public Object {
    SceneVehicle(const SceneVehicle&) = delete;
    SceneVehicle& operator = (const SceneVehicle&) = delete;
public:
    using CreateExternals = std::function<void(const std::string&, ExternalsMode, const std::unordered_map<ControlSource, Skills>&)>;
    SceneVehicle(
        DeleteNodeMutex& delete_node_mutex,
        std::string scene_node_name,
        SceneNode& scene_node,
        RigidBodyVehicle& rb);
    ~SceneVehicle();
    void create_externals(
        const std::string& player_name,
        ExternalsMode externals_mode,
        const std::unordered_map<ControlSource, Skills>& skills) const;
    void set_create_externals(const CreateExternals& create_externals);
    std::string& scene_node_name();
    const std::string& scene_node_name() const;
    SceneNode& scene_node();
    const SceneNode& scene_node() const;
    RigidBodyVehicle& rb();
    const RigidBodyVehicle& rb() const;
    DestructionObservers destruction_observers;
private:
    DeleteNodeMutex& delete_node_mutex_;
    std::string scene_node_name_;
    SceneNode& scene_node_;
    RigidBodyVehicle& rb_;
    CreateExternals create_externals_;
};

}
