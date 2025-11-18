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
#include <Mlib/Scene/Load_Scene_Functions/Fast_Macros/Create_Generic_Avatar.hpp>
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
    IoVerbosity verbosity,
    RemoteSceneObjectType type,
    std::string initial,
    std::string node_suffix,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : type_{ type }
    , initial_{ std::move(initial) }
    , node_suffix_{ std::move(node_suffix) }
    , rb_{ rb.ptr() }
    , physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
    , rb_on_destroy_{ rb->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteRigidBodyVehicle";
    }
    rb_on_destroy_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteRigidBodyVehicle::~RemoteRigidBodyVehicle() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteRigidBodyVehicle";
    }
    if (rb_ == nullptr) {
        verbose_abort("RemoteRigidBodyVehicle: Rigid body is null");
    }
    if (!rb_->remote_object_id_.has_value()) {
        verbose_abort("RemoteRigidBodyVehicle: Rigid body has no remote object ID");
    }
    if (physics_scene_->remote_scene_ == nullptr) {
        verbose_abort("RemoteRigidBodyVehicle: Remote scene is null");
    }
    physics_scene_->remote_scene_->created_at_remote_site.rigid_bodies.erase(rb_->node_name_);
    physics_scene_->remote_scene_->try_remove(*rb_->remote_object_id_);
    if (rb_ != nullptr) {
        global_object_pool.remove(rb_.release());
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemoteRigidBodyVehicle> RemoteRigidBodyVehicle::try_create_from_stream(
    RemoteSceneObjectType type,
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
    std::optional<RemoteSiteId> owner_site_id;
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        owner_site_id = reader.read_binary<RemoteSiteId>("owner_site_id");
    }
    auto end = reader.read_binary<uint32_t>("inverted scene object type");
    if (end != ~(uint32_t)type) {
        THROW_OR_ABORT("Invalid rigid body vehicle end (0)");
    }
    if ((type != RemoteSceneObjectType::RIGID_BODY_CAR) &&
        (type != RemoteSceneObjectType::RIGID_BODY_AVATAR))
    {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Unexpected remote scene object type");
    }
    if (physics_scene.remote_scene_ == nullptr) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Remote scene is null");
    }
    auto local_site_id = physics_scene.remote_scene_->local_site_id();
    if (!any(transmitted_fields & RigidBodyTransmittedFields::INITIAL) ||
        !any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP) ||
        (remote_object_id.site_id == local_site_id))
    {
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
    std::string node_suffix;
    VariableAndHash<std::string> node_name;
    if (type == RemoteSceneObjectType::RIGID_BODY_CAR) {
        node_suffix = initial.at<std::string>("tesuffix");
        node_name = VariableAndHash<std::string>{"car_node" + node_suffix};
    } else if (type == RemoteSceneObjectType::RIGID_BODY_AVATAR) {
        node_suffix = initial.at<std::string>("suffix");
        node_name = VariableAndHash<std::string>{"human_node" + node_suffix};
    } else {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Unknoen scene object type");
    }
    physics_scene.remote_scene_->created_at_remote_site.rigid_bodies.add(node_name);
    physics_scene.scene_.add_root_node(
        node_name,
        std::move(node),
        RenderingDynamics::MOVING,
        RenderingStrategies::OBJECT);
    if (type == RemoteSceneObjectType::RIGID_BODY_CAR) {
        CreateGenericCar(physics_scene).execute(initial);
    } else if (type == RemoteSceneObjectType::RIGID_BODY_AVATAR) {
        CreateGenericAvatar(physics_scene, physics_scene.macro_line_executor_).execute(initial);
    } else {
        THROW_OR_ABORT("RemoteRigidBodyVehicle: Unknoen scene object type");
    }
    auto& rb = get_rigid_body_vehicle(pnode);
    rb.rbp_.v_com_ = v_com;
    rb.rbp_.w_ = w;
    rb.remote_object_id_ = remote_object_id;
    if (owner_site_id.has_value()) {
        rb.owner_site_id_ = *owner_site_id;
    }
    return {
        global_object_pool.create<RemoteRigidBodyVehicle>(
            CURRENT_SOURCE_LOCATION,
            verbosity,
            type,
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
    if (rb_ == nullptr) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::read: Rigid body is destroyed");
    }
    auto reader = BinaryReader{istr, verbosity_};
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != type_) {
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
    auto end = reader.read_binary<uint32_t>("inverted scene object type");
    if (end != ~(uint32_t)type_) {
        THROW_OR_ABORT("Invalid rigid body vehicle end (1)");
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
    if (rb_ == nullptr) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::write: Rigid body is destroyed");
    }
    auto transmitted_fields = TransmittedFields::NONE;
    if (!any(known_fields & RigidBodyKnownFields::INITIAL)) {
        transmitted_fields |= RigidBodyTransmittedFields::INITIAL;
    }
    if (any(proxy_tasks & ProxyTasks::SEND_OWNERSHIP)) {
        transmitted_fields |= RigidBodyTransmittedFields::OWNERSHIP;
    }
    transmission_history_writer.write(ostr, remote_object_id, transmitted_fields);
    auto writer = BinaryWriter{ostr};
    writer.write_binary(type_, "rigid body vehicle");
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
    writer.write_binary(~(uint32_t)type_, "inverted rigid body vehicle");
}

DanglingBaseClassRef<RigidBodyVehicle> RemoteRigidBodyVehicle::rb() {
    if (rb_ == nullptr) {
        THROW_OR_ABORT("RemoteRigidBodyVehicle::rb: Rigid body is destroyed");
    }
    return *rb_;
}

const std::string& RemoteRigidBodyVehicle::node_suffix() const {
    return node_suffix_;
}
