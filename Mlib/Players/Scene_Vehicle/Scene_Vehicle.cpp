#include "Scene_Vehicle.hpp"
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SceneVehicle::SceneVehicle(
    DeleteNodeMutex& delete_node_mutex,
    std::string scene_node_name,
    SceneNode& scene_node,
    RigidBodyVehicle& rb)
: destruction_observers{*this},
  delete_node_mutex_{delete_node_mutex},
  scene_node_name_{std::move(scene_node_name)},
  scene_node_{scene_node},
  rb_{rb}
{}

SceneVehicle::~SceneVehicle() = default;

void SceneVehicle::create_externals(
    const std::string& player_name,
    ExternalsMode externals_mode,
    const std::unordered_map<ControlSource, Skills>& skills) const
{
    if (!create_externals_) {
        THROW_OR_ABORT("create_externals not set");
    }
    create_externals_(player_name, externals_mode, skills);
}

void SceneVehicle::set_create_externals(const CreateExternals& create_externals)
{
    if (create_externals_) {
        THROW_OR_ABORT("create_externals already set");
    }
    create_externals_ = create_externals;
}

std::string& SceneVehicle::scene_node_name() {
    delete_node_mutex_.notify_reading();
    return scene_node_name_;
}

const std::string& SceneVehicle::scene_node_name() const {
    delete_node_mutex_.notify_reading();
    return scene_node_name_;
}

SceneNode& SceneVehicle::scene_node() {
    delete_node_mutex_.notify_reading();
    return scene_node_;
}

const SceneNode& SceneVehicle::scene_node() const {
    delete_node_mutex_.notify_reading();
    return scene_node_;
}

RigidBodyVehicle& SceneVehicle::rb() {
    delete_node_mutex_.notify_reading();
    return rb_;
}

const RigidBodyVehicle& SceneVehicle::rb() const {
    delete_node_mutex_.notify_reading();
    return rb_;
}
