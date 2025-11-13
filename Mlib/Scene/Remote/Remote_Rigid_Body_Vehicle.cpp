#include "Remote_Rigid_Body_Vehicle.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
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
    std::string node_suffix,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb)
    : initial_{ std::move(initial) }
    , node_suffix_{ std::move(node_suffix) }
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

DanglingBaseClassPtr<RemoteRigidBodyVehicle> RemoteRigidBodyVehicle::try_create_from_stream(
    ObjectPool& object_pool,
    PhysicsScene& physics_scene,
    std::istream& istr,
    IoVerbosity verbosity)
{
    auto reader = BinaryReader{istr, verbosity};
    auto compression = reader.read_binary<ObjectCompression>("object compression");
    if (compression == ObjectCompression::NONE) {
        // Continue
    } else if (compression == ObjectCompression::INCREMENTAL) {
        return nullptr;
    } else {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::try_create_from_stream: Unknown compression mode");
    }
    auto initial_str = reader.read_string("initial rigid body");
    auto initial_json = nlohmann::json::parse(initial_str);
    auto initial = JsonView{ initial_json };
    auto position = reader.read_binary<EFixedArray<ScenePos, 3>>("position");
    auto rotation = reader.read_binary<EFixedArray<SceneDir, 3>>("rotation");
    auto v_com = reader.read_binary<EFixedArray<SceneDir, 3>>("v_com");
    auto w = reader.read_binary<EFixedArray<SceneDir, 3>>("w");
    auto node = make_unique_scene_node(
        position,
        rotation,
        1.f, // scale
        PoseInterpolationMode::ENABLED,
        SceneNodeDomain::RENDER | SceneNodeDomain::PHYSICS,
        ViewableRemoteObject::all());
    auto pnode = node.ref(CURRENT_SOURCE_LOCATION);
    auto node_suffix = initial.at<std::string>("tesuffix");
    auto node_name = VariableAndHash<std::string>{"car_node" + node_suffix};
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
    return {
        object_pool.create<RemoteRigidBodyVehicle>(
            CURRENT_SOURCE_LOCATION,
            object_pool,
            verbosity,
            std::move(initial_str),
            std::move(node_suffix),
            DanglingBaseClassRef<RigidBodyVehicle>{
                get_rigid_body_vehicle(pnode),
                CURRENT_SOURCE_LOCATION}),
        CURRENT_SOURCE_LOCATION};
}

void RemoteRigidBodyVehicle::read(std::istream& istr) {
    auto reader = BinaryReader{istr, verbosity_};
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::RIGID_BODY_VEHICLE) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Unexpected scene object type");
    }
    auto compression = reader.read_binary<ObjectCompression>("object compression");
    if (compression == ObjectCompression::NONE) {
        auto initial_len = reader.read_binary<uint32_t>("initial rigid body length");
        reader.seek_relative_positive(initial_len);
    } else if (compression != ObjectCompression::INCREMENTAL) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Unknown compression mode");
    }
    auto position = reader.read_binary<EFixedArray<ScenePos, 3>>("position");
    auto rotation = reader.read_binary<EFixedArray<SceneDir, 3>>("rotation");
    auto v_com = reader.read_binary<EFixedArray<SceneDir, 3>>("v_com");
    auto w = reader.read_binary<EFixedArray<SceneDir, 3>>("w");
    rb_->rbp_.set_pose(tait_bryan_angles_2_matrix(rotation), position);
    rb_->rbp_.v_com_ = v_com;
    rb_->rbp_.w_ = w;
}

void RemoteRigidBodyVehicle::write(std::ostream& ostr, ObjectCompression compression) {
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::RIGID_BODY_VEHICLE, "rigid body vehicle");
    writer.write_binary(compression, "rigi body compression");
    if (compression == ObjectCompression::NONE) {
        writer.write_string(initial_, "initial rigid body");
    }
    writer.write_binary(rb_->rbp_.abs_position(), "position");
    writer.write_binary(matrix_2_tait_bryan_angles(rb_->rbp_.rotation_), "rotation");
    writer.write_binary(rb_->rbp_.v_com_, "v_com");
    writer.write_binary(rb_->rbp_.w_, "w");
}

DanglingBaseClassRef<RigidBodyVehicle> RemoteRigidBodyVehicle::rb() {
    return rb_;
}

const std::string& RemoteRigidBodyVehicle::node_suffix() const {
    return node_suffix_;
}
