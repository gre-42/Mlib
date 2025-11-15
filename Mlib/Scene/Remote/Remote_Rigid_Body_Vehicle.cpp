#include "Remote_Rigid_Body_Vehicle.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
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

enum class RigidBodyTransmittedFields: uint32_t {
    INITIAL = (uint32_t)TransmittedFields::END,
    OWNERSHIP = (uint32_t)TransmittedFields::END << 1
};

inline TransmittedFields operator & (TransmittedFields a, RigidBodyTransmittedFields b) {
    return (TransmittedFields)((uint32_t)a & (uint32_t)b);
}

inline TransmittedFields operator | (TransmittedFields a, RigidBodyTransmittedFields b) {
    return (TransmittedFields)((uint32_t)a | (uint32_t)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, RigidBodyTransmittedFields b) {
    (uint32_t&)a |= (uint32_t)b;
    return a;
}

enum class RigidBodyKnownFields: uint32_t {
    INITIAL = (uint32_t)KnownFields::END
};

inline KnownFields operator & (KnownFields a, RigidBodyKnownFields b) {
    return (KnownFields)((uint32_t)a & (uint32_t)b);
}

RemoteRigidBodyVehicle::RemoteRigidBodyVehicle(
    ObjectPool& object_pool,
    IoVerbosity verbosity,
    std::string initial,
    std::string node_suffix,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : initial_{ std::move(initial) }
    , node_suffix_{ std::move(node_suffix) }
    , rb_{ rb }
    , physics_scene_{ physics_scene }
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
    TransmittedFields transmitted_fields,
    const RemoteObjectId& remote_object_id,
    IoVerbosity verbosity)
{
    auto reader = BinaryReader{istr, verbosity};
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        TransmittedFields::END |
        RigidBodyTransmittedFields::INITIAL |
        RigidBodyTransmittedFields::OWNERSHIP)))
    {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::try_create_from_stream: Unknown transmitted fields");
    }
    auto initial_str = std::string("");
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        initial_str = reader.read_string("initial rigid body");
    }
    auto position = reader.read_binary<EFixedArray<ScenePos, 3>>("position");
    auto rotation = reader.read_binary<EFixedArray<SceneDir, 3>>("rotation");
    auto v_com = reader.read_binary<EFixedArray<SceneDir, 3>>("v_com");
    auto w = reader.read_binary<EFixedArray<SceneDir, 3>>("w");
    auto owner_site_id = reader.read_binary<RemoteSiteId>("owner_site_id");
    if (!any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        return nullptr;
    }
    auto initial_json = nlohmann::json::parse(initial_str);
    auto initial = JsonView{ initial_json };
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
    rb.remote_object_id_ = remote_object_id;
    rb.owner_site_id_ = owner_site_id;
    return {
        object_pool.create<RemoteRigidBodyVehicle>(
            CURRENT_SOURCE_LOCATION,
            object_pool,
            verbosity,
            std::move(initial_str),
            std::move(node_suffix),
            DanglingBaseClassRef<RigidBodyVehicle>{get_rigid_body_vehicle(pnode), CURRENT_SOURCE_LOCATION},
            DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION}),
        CURRENT_SOURCE_LOCATION};
}

void RemoteRigidBodyVehicle::read(
    std::istream& istr,
    TransmittedFields transmitted_fields)
{
    auto reader = BinaryReader{istr, verbosity_};
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::RIGID_BODY_VEHICLE) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Unexpected scene object type");
    }
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        auto initial_len = reader.read_binary<uint32_t>("initial rigid body length");
        reader.seek_relative_positive(initial_len);
    }
    auto position = reader.read_binary<EFixedArray<ScenePos, 3>>("position");
    auto rotation = reader.read_binary<EFixedArray<SceneDir, 3>>("rotation");
    auto v_com = reader.read_binary<EFixedArray<SceneDir, 3>>("v_com");
    auto w = reader.read_binary<EFixedArray<SceneDir, 3>>("w");
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        rb_->owner_site_id_ = reader.read_binary<RemoteSiteId>("owner_site_id");
    }
    if (physics_scene_->remote_scene_ == nullptr) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Remote scene is null");
    }
    if (!rb_->owner_site_id_.has_value()) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Owner site ID not set");
    }
    if (*rb_->owner_site_id_ != physics_scene_->remote_scene_->local_site_id()) {
        rb_->rbp_.set_pose(tait_bryan_angles_2_matrix(rotation), position);
        rb_->rbp_.v_com_ = v_com;
        rb_->rbp_.w_ = w;
    }
}

void RemoteRigidBodyVehicle::write(
    std::ostream& ostr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    TransmissionHistoryWriter& transmission_history_writer)
{
    auto transmitted_fields = TransmittedFields::NONE;
    if (!any(known_fields & RigidBodyKnownFields::INITIAL)) {
        transmitted_fields |= RigidBodyTransmittedFields::INITIAL;
    }
    if (any(proxy_tasks & ProxyTasks::SEND_OWNERSHIP)) {
        transmitted_fields |= RigidBodyTransmittedFields::OWNERSHIP;
    }
    transmission_history_writer.write(ostr, remote_object_id, transmitted_fields);
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::RIGID_BODY_VEHICLE, "rigid body vehicle");
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        writer.write_string(initial_, "initial rigid body");
    }
    writer.write_binary(rb_->rbp_.abs_position(), "position");
    writer.write_binary(matrix_2_tait_bryan_angles(rb_->rbp_.rotation_), "rotation");
    writer.write_binary(rb_->rbp_.v_com_, "v_com");
    writer.write_binary(rb_->rbp_.w_, "w");
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        if (!rb_->owner_site_id_.has_value()) {
            THROW_OR_ABORT("Owner site ID not set");
        }
        writer.write_binary(*rb_->owner_site_id_, "owner site ID");
    }
}

DanglingBaseClassRef<RigidBodyVehicle> RemoteRigidBodyVehicle::rb() {
    return rb_;
}

const std::string& RemoteRigidBodyVehicle::node_suffix() const {
    return node_suffix_;
}
