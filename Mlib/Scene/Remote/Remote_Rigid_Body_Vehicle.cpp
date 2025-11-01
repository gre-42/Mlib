#include "Remote_Rigid_Body_Vehicle.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Env.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Remote/Object_Compression.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Fast_Macros/Create_Generic_Car.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <nlohmann/json.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<ScenePos, 3>) == 3 * 8);
static_assert(sizeof(FixedArray<SceneDir, 3>) == 3 * 4);

RemoteRigidBodyVehicle::RemoteRigidBodyVehicle(
    ObjectPool& object_pool,
    IoVerbosity verbosity,
    std::string initial,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb)
    : initial_{ std::move(initial) }
    , rb_{ rb }
    , verbosity_{ verbosity }
    , rb_on_destroy_{ rb->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteRigidBodyVehicle";
    }
    rb_on_destroy_.add([&o=object_pool, this](){ o.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteRigidBodyVehicle::~RemoteRigidBodyVehicle() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteRigidBodyVehicle";
    }
    on_destroy.clear();
}

std::unique_ptr<RemoteRigidBodyVehicle> RemoteRigidBodyVehicle::try_create_from_stream(
    ObjectPool& object_pool,
    PhysicsScene& physics_scene,
    std::istream& istr,
    IoVerbosity verbosity)
{
    auto compression = read_binary<ObjectCompression>(istr, "object compression", verbosity);
    if (compression == ObjectCompression::NONE) {
        // Continue
    } else if (compression == ObjectCompression::INCREMENTAL) {
        return nullptr;
    } else {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::try_create_from_stream: Unknown compression mode");
    }
    auto initial_len = read_binary<uint32_t>(istr, "initial rigid body length", verbosity);
    auto initial_str = read_string(istr, initial_len, "initial rigid body", verbosity);
    auto initial_json = nlohmann::json::parse(initial_str);
    auto initial = JsonView{ initial_json };
    auto position = read_binary<EFixedArray<ScenePos, 3>>(istr, "position", verbosity);
    auto rotation = read_binary<EFixedArray<SceneDir, 3>>(istr, "rotation", verbosity);
    auto v_com = read_binary<EFixedArray<SceneDir, 3>>(istr, "v_com", verbosity);
    auto w = read_binary<EFixedArray<SceneDir, 3>>(istr, "w", verbosity);
    auto node = make_unique_scene_node(
        position,
        rotation,
        1.f, // scale
        PoseInterpolationMode::ENABLED,
        SceneNodeDomain::RENDER | SceneNodeDomain::PHYSICS,
        UINT32_MAX); // user_id
    auto pnode = node.ref(CURRENT_SOURCE_LOCATION);
    auto node_name = VariableAndHash<std::string>{"car_node" + initial.at<std::string>("tesuffix")};
    physics_scene.remote_scene_->created_at_remote_site.rigid_bodies.add(node_name);
    physics_scene.scene_.add_root_node(
        node_name,
        std::move(node),
        RenderingDynamics::MOVING,
        RenderingStrategies::OBJECT);
    CreateGenericCar(physics_scene).execute(initial);
    auto& rb = get_rigid_body_vehicle(pnode);
    rb.rbp_.v_com_ = v_com;
    rb.rbp_.w_ = w;
    return std::make_unique<RemoteRigidBodyVehicle>(
        object_pool,
        verbosity,
        std::move(initial_str),
        DanglingBaseClassRef<RigidBodyVehicle>{
            get_rigid_body_vehicle(pnode),
            CURRENT_SOURCE_LOCATION});
}

void RemoteRigidBodyVehicle::read(std::istream& istr) {
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    if (type != RemoteSceneObjectType::RIGID_BODY_VEHICLE) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Unexpected scene object type");
    }
    auto compression = read_binary<ObjectCompression>(istr, "object compression", verbosity_);
    if (compression == ObjectCompression::NONE) {
        auto initial_len = read_binary<uint32_t>(istr, "initial rigid body length", verbosity_);
        seek_relative_positive(istr, initial_len, verbosity_);
    } else if (compression != ObjectCompression::INCREMENTAL) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Unknown compression mode");
    }
    auto position = read_binary<EFixedArray<ScenePos, 3>>(istr, "position", verbosity_);
    auto rotation = read_binary<EFixedArray<SceneDir, 3>>(istr, "rotation", verbosity_);
    auto v_com = read_binary<EFixedArray<SceneDir, 3>>(istr, "v_com", verbosity_);
    auto w = read_binary<EFixedArray<SceneDir, 3>>(istr, "w", verbosity_);
    rb_->rbp_.set_pose(tait_bryan_angles_2_matrix(rotation), position);
    rb_->rbp_.v_com_ = v_com;
    rb_->rbp_.w_ = w;
}

void RemoteRigidBodyVehicle::write(std::ostream& ostr, ObjectCompression compression) {
    write_binary(ostr, RemoteSceneObjectType::RIGID_BODY_VEHICLE, "rigid body vehicle");
    write_binary(ostr, compression, "rigi body compression");
    if (compression == ObjectCompression::NONE) {
        write_binary<uint32_t>(ostr, integral_cast<uint32_t>(initial_.length()), "initial rigid body length");
        write_iterable(ostr, initial_, "initial rigid body");
    }
    write_binary(ostr, rb_->rbp_.abs_position(), "position");
    write_binary(ostr, matrix_2_tait_bryan_angles(rb_->rbp_.rotation_), "rotation");
    write_binary(ostr, rb_->rbp_.v_com_, "v_com");
    write_binary(ostr, rb_->rbp_.w_, "w");
}
