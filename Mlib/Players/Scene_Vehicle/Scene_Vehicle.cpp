#include "Scene_Vehicle.hpp"
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

SceneVehicle::SceneVehicle(
    DeleteNodeMutex& delete_node_mutex,
    VariableAndHash<std::string> scene_node_name,
    const DanglingRef<SceneNode>& scene_node,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb)
    : delete_node_mutex_{ delete_node_mutex }
    , on_scene_node_destroyed_{ scene_node->on_destroy, CURRENT_SOURCE_LOCATION }
    , on_rigid_body_destroyed_{ rb->on_destroy, CURRENT_SOURCE_LOCATION }
    , scene_node_name_{ std::move(scene_node_name) }
    , scene_node_{ scene_node.ptr().set_loc(CURRENT_SOURCE_LOCATION) }
    , rb_{ rb.ptr().set_loc(CURRENT_SOURCE_LOCATION) }
{
    on_scene_node_destroyed_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
    on_rigid_body_destroyed_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

SceneVehicle::~SceneVehicle() {
    scene_node_ = nullptr;
    rb_ = nullptr;
    on_destroy.clear();
}

void SceneVehicle::create_vehicle_externals(
    uint32_t user_id,
    const std::string& user_name,
    const std::string& player_name,
    ExternalsMode externals_mode,
    const std::string& behavior) const
{
    if (!create_vehicle_externals_) {
        THROW_OR_ABORT("create_vehicle_externals not set");
    }
    create_vehicle_externals_(user_id, user_name, player_name, externals_mode, behavior);
}

void SceneVehicle::create_vehicle_internals(
    uint32_t user_id,
    const std::string& user_name,
    const std::string& player_name,
    ExternalsMode externals_mode,
    const SkillMap& skills,
    const std::string& behavior,
    const InternalsMode& internals_mode) const
{
    if (!create_vehicle_internals_) {
        THROW_OR_ABORT("create_vehicle_internals not set");
    }
    create_vehicle_internals_(user_id, user_name, player_name, externals_mode, skills, behavior, internals_mode);
}

void SceneVehicle::set_create_vehicle_externals(
    const CreateVehicleExternals& create_vehicle_externals)
{
    if (create_vehicle_externals_) {
        THROW_OR_ABORT("create_vehicle_externals already set");
    }
    create_vehicle_externals_ = create_vehicle_externals;
}

void SceneVehicle::set_create_vehicle_internals(
    const CreateRoleExternals& create_vehicle_internals)
{
    if (create_vehicle_internals_) {
        THROW_OR_ABORT("create_vehicle_internals already set");
    }
    create_vehicle_internals_ = create_vehicle_internals;
}

VariableAndHash<std::string>& SceneVehicle::scene_node_name() {
    return scene_node_name_;
}

const VariableAndHash<std::string>& SceneVehicle::scene_node_name() const {
    return scene_node_name_;
}

DanglingRef<SceneNode> SceneVehicle::scene_node() {
    return *scene_node_;
}

DanglingRef<const SceneNode> SceneVehicle::scene_node() const {
    return *scene_node_;
}

DanglingBaseClassRef<RigidBodyVehicle> SceneVehicle::rb() {
    return *rb_;
}

DanglingBaseClassRef<const RigidBodyVehicle> SceneVehicle::rb() const {
    return *rb_;
}
