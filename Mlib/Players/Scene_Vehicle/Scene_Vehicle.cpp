#include "Scene_Vehicle.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <stdexcept>

using namespace Mlib;

SceneVehicle::SceneVehicle(
    VariableAndHash<std::string> scene_node_name,
    const DanglingBaseClassRef<SceneNode>& scene_node,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb)
    : on_scene_node_destroyed_{ scene_node->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
    , on_rigid_body_destroyed_{ rb->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
    , scene_node_name_{ std::move(scene_node_name) }
    , scene_node_{ scene_node.ptr().set_loc(CURRENT_SOURCE_LOCATION) }
    , rb_{ rb.ptr().set_loc(CURRENT_SOURCE_LOCATION) }
{
    on_scene_node_destroyed_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    on_rigid_body_destroyed_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

SceneVehicle::~SceneVehicle() {
    on_destroy.clear();
    scene_node_ = nullptr;
    rb_ = nullptr;
}

void SceneVehicle::create_vehicle_externals(
    const Player& player,
    ExternalsMode externals_mode) const
{
    if (!create_vehicle_externals_) {
        throw std::runtime_error("create_vehicle_externals not set");
    }
    create_vehicle_externals_(player, externals_mode);
}

void SceneVehicle::create_vehicle_internals(
    const Player& player,
    const InternalsMode& internals_mode) const
{
    if (!create_vehicle_internals_) {
        throw std::runtime_error("create_vehicle_internals not set");
    }
    create_vehicle_internals_(player, internals_mode);
}

void SceneVehicle::set_create_vehicle_externals(
    const CreateVehicleExternals& create_vehicle_externals)
{
    if (create_vehicle_externals_) {
        throw std::runtime_error("create_vehicle_externals already set");
    }
    create_vehicle_externals_ = create_vehicle_externals;
}

void SceneVehicle::set_create_vehicle_internals(
    const CreateRoleExternals& create_vehicle_internals)
{
    if (create_vehicle_internals_) {
        throw std::runtime_error("create_vehicle_internals already set");
    }
    create_vehicle_internals_ = create_vehicle_internals;
}

VariableAndHash<std::string>& SceneVehicle::scene_node_name() {
    return scene_node_name_;
}

const VariableAndHash<std::string>& SceneVehicle::scene_node_name() const {
    return scene_node_name_;
}

DanglingBaseClassRef<SceneNode> SceneVehicle::scene_node() {
    return *scene_node_;
}

DanglingBaseClassRef<const SceneNode> SceneVehicle::scene_node() const {
    return *scene_node_;
}

ShutdownPhase SceneVehicle::scene_node_shutdown_phase() const {
    if (scene_node_ == nullptr) {
        throw std::runtime_error("SceneVehicle::scene_node_shutdown_phase called on null scene node");
    }
    return scene_node_->shutdown_phase();
}

DanglingBaseClassRef<RigidBodyVehicle> SceneVehicle::rb() {
    return *rb_;
}

DanglingBaseClassRef<const RigidBodyVehicle> SceneVehicle::rb() const {
    return *rb_;
}
