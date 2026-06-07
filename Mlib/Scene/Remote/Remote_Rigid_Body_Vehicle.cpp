#include "Remote_Rigid_Body_Vehicle.hpp"
#include <Mlib/Components/Rigid_Body_Vehicle.hpp>
#include <Mlib/Geometry/Coordinates/To_Tait_Bryan_Angles.hpp>
#include <Mlib/Geometry/Instance/Rendering_Dynamics.hpp>
#include <Mlib/Json/Base.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle_Flags.hpp>
#include <Mlib/Remote/Incremental_Objects/Known_Fields.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Remote_Check.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Fast_Macros/Create_Generic_Avatar.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Fast_Macros/Create_Generic_Car.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Remote/Avatar_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Remote/Car_Parameters.hpp>
#include <Mlib/Scene/Load_Scene_Functions/Remote/Vehicle_Parameters.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Coordinate_Deserialization.hpp>
#include <Mlib/Scene/Remote/Coordinate_Serialization.hpp>
#include <Mlib/Scene/Remote/Remote_Privileges.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Priority.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene_Config/Interpolation_Thresholds.hpp>
#include <Mlib/Scene_Config/Remote_Integers.hpp>
#include <Mlib/Scene_Graph/Containers/Scene.hpp>
#include <Mlib/Scene_Graph/Elements/Make_Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Rendering_Strategies.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Elements/Scene_Time.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<ScenePos, 3>) == 3 * 8);
static_assert(sizeof(FixedArray<SceneDir, 3>) == 3 * 4);

enum class RigidBodyTransmittedFields: TransmittedFieldsType {
    NONZERO = (TransmittedFieldsType)TransmittedFields::END,
    INITIAL = (TransmittedFieldsType)TransmittedFields::END << 1,
    OWNERSHIP = (TransmittedFieldsType)TransmittedFields::END << 2,
};

inline TransmittedFields operator & (TransmittedFields a, RigidBodyTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a & (TransmittedFieldsType)b);
}

inline TransmittedFields operator | (TransmittedFields a, RigidBodyTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a | (TransmittedFieldsType)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, RigidBodyTransmittedFields b) {
    (TransmittedFieldsType&)a |= (TransmittedFieldsType)b;
    return a;
}

RemoteRigidBodyVehicle::RemoteRigidBodyVehicle(
    IoVerbosity verbosity,
    RemoteSceneObjectType type,
    nlohmann::json initial,
    std::string node_suffix,
    const DanglingBaseClassRef<RigidBodyVehicle>& rb,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : type_{ type }
    , initial_( std::move(initial) )
    , node_suffix_{ std::move(node_suffix) }
    , rb_{ rb.ptr() }
    , physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
    , rb_on_destroy_{ rb->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
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
    physics_scene_->remote_scene_->try_remove(*rb_->remote_object_id_);
    if (rb_ != nullptr) {
        global_object_pool.remove(rb_.release());
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemoteRigidBodyVehicle> RemoteRigidBodyVehicle::try_create_from_stream(
    RemoteSceneObjectType type,
    PhysicsScene& physics_scene,
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields,
    const RemoteObjectId& remote_object_id,
    IoVerbosity verbosity)
{
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        TransmittedFields::END |
        RigidBodyTransmittedFields::INITIAL |
        RigidBodyTransmittedFields::OWNERSHIP |
        RigidBodyTransmittedFields::NONZERO)))
    {
        throw std::runtime_error("RemoteRigidBodyVehicle::try_create_from_stream: Unknown transmitted fields");
    }
    auto initial = nlohmann::json::object();
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        [&](){
            switch (type) {
            case RemoteSceneObjectType::RIGID_BODY_CAR:
                initial[CarParameters::asset_id] = reader.read_string<StringLengthType>("asset_id");
                initial[CarParameters::suffix] = reader.read_string<StringLengthType>("suffix");
                initial[CarParameters::if_car_body_renderable_style] = reader.read_bool_bit("if_car_body_renderable_style");
                initial[CarParameters::if_damageable] = reader.read_bool_bit("if_damageable");
                initial[CarParameters::parking_brake_pulled] = reader.read_bool_bit("parking_brake_pulled");
                initial[CarParameters::color] = reader.read_binary<EFixedArray<float, 3>>("color");
                initial[CarParameters::velocity_error_std] = reader.read_binary<float>("velocity_error_std");
                initial[CarParameters::error_alpha] = reader.read_binary<float>("error_alpha");
                initial[CarParameters::yaw_error_std] = reader.read_binary<float>("yaw_error_std");
                initial[CarParameters::pitch_error_std] = reader.read_binary<float>("pitch_error_std");
                initial[CarParameters::mute] = false;
                initial[CarParameters::if_with_graphics] = true;
                initial[CarParameters::if_with_physics] = true;
                return;
            case RemoteSceneObjectType::RIGID_BODY_AVATAR:
                initial[AvatarParameters::asset_id] = reader.read_string<StringLengthType>("asset_id");
                initial[AvatarParameters::suffix] = reader.read_string<StringLengthType>("suffix");
                initial[AvatarParameters::if_human_style] = reader.read_bool_bit("if_human_style");
                initial[AvatarParameters::if_damageable] = reader.read_bool_bit("if_damageable");
                initial[AvatarParameters::parking_brake_pulled] = reader.read_bool_bit("parking_brake_pulled");
                initial[AvatarParameters::color] = reader.read_binary<EFixedArray<float, 3>>("color");
                initial[AvatarParameters::velocity_error_std] = reader.read_binary<float>("velocity_error_std");
                initial[AvatarParameters::error_alpha] = reader.read_binary<float>("error_alpha");
                initial[AvatarParameters::locked_on_angle] = reader.read_binary<float>("locked_on_angle");
                initial[AvatarParameters::yaw_error_std] = reader.read_binary<float>("yaw_error_std");
                initial[AvatarParameters::pitch_error_std] = reader.read_binary<float>("pitch_error_std");
                initial[AvatarParameters::pitch_min] = reader.read_binary<float>("pitch_min");
                initial[AvatarParameters::pitch_max] = reader.read_binary<float>("pitch_max");
                initial[AvatarParameters::dpitch_max] = reader.read_binary<float>("dpitch_max");
                initial[AvatarParameters::dyaw_max] = reader.read_binary<float>("dyaw_max");
                initial[AvatarParameters::steering_multiplier] = reader.read_binary<float>("steering_multiplier");
                initial[AvatarParameters::animation_resource_wo_gun] = reader.read_string<StringLengthType>("animation_resource_wo_gun");
                initial[AvatarParameters::animation_resource_w_gun] = reader.read_string<StringLengthType>("animation_resource_w_gun");
                initial[AvatarParameters::y_fov] = reader.read_binary<float>("y_fov");
                initial[AvatarParameters::near_plane] = reader.read_binary<float>("near_plane");
                initial[AvatarParameters::far_plane] = reader.read_binary<float>("far_plane");
                initial[AvatarParameters::with_gun] = reader.read_binary<bool>("with_gun");
                initial[AvatarParameters::mute] = false;
                initial[AvatarParameters::if_with_graphics] = true;
                initial[AvatarParameters::if_with_physics] = true;
                return;
            case RemoteSceneObjectType::REMOTE_USERS:
            case RemoteSceneObjectType::PLAYER:
            case RemoteSceneObjectType::COUNTDOWN:
                throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
            }
            throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
        }();
    }
    auto position = deserialize_position(reader, "position");
    auto v_com = deserialize_direction(reader, "v_com");
    FixedArray<SceneDir, 3> rotation = uninitialized;
    FixedArray<SceneDir, 3> w = uninitialized;
    [&](){
        switch (type) {
        case RemoteSceneObjectType::RIGID_BODY_CAR:
            rotation = deserialize_angles(reader, "rotation");
            w = deserialize_direction(reader, "w");
            return;
        case RemoteSceneObjectType::RIGID_BODY_AVATAR:
            rotation = {0.f, deserialize_angle(reader, "avatar yaw"), 0.f};
            w = {0.f, reader.read_binary<SceneDir>("w"), 0.f};
            return;
        case RemoteSceneObjectType::REMOTE_USERS:
        case RemoteSceneObjectType::PLAYER:
        case RemoteSceneObjectType::COUNTDOWN:
            throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
        }
        throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
    }();
    auto flags = reader.read_bits<RigidBodyVehicleFlags>(RIGID_BODY_VEHICLE_FLAGS_NBITS, "rigid body flags");
    std::optional<RemoteSiteId> owner_site_id;
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        owner_site_id = reader.read_binary<RemoteSiteId>("owner_site_id");
    }
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted scene object type");
        if (end != ~type) {
            throw std::runtime_error("Invalid rigid body vehicle end (0)");
        }
    }
    if ((type != RemoteSceneObjectType::RIGID_BODY_CAR) &&
        (type != RemoteSceneObjectType::RIGID_BODY_AVATAR))
    {
        throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected remote scene object type");
    }
    if (physics_scene.remote_scene_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle: Remote scene is null");
    }
    auto local_site_id = physics_scene.remote_scene_->local_site_id();
    if (!any(transmitted_fields & RigidBodyTransmittedFields::INITIAL) ||
        !any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP) ||
        (remote_object_id.site_id == local_site_id))
    {
        return nullptr;
    }
    #ifdef WITHOUT_GRAPHICS
    initial[VehicleParameters::if_with_graphics] = false;
    #endif
    auto initial_view = JsonView{ initial };
    auto node = make_unique_scene_node(
        position,
        rotation,
        1.f, // scale
        physics_scene.dynamic_world_.try_get_time(),
        PoseInterpolationMode::ENABLED,
        SceneNodeDomain::RENDER | SceneNodeDomain::PHYSICS,
        ViewableRemoteObject::all());
    auto pnode = node.ref(CURRENT_SOURCE_LOCATION);
    std::string node_suffix = initial_view.at<std::string>(VehicleParameters::suffix);
    VariableAndHash<std::string> node_name;
    auto let = nlohmann::json::object();
    [&](){
        switch (type) {
        case RemoteSceneObjectType::RIGID_BODY_CAR:
            node_name = VariableAndHash<std::string>{"car_node" + node_suffix};
            return;
        case RemoteSceneObjectType::RIGID_BODY_AVATAR:
            node_name = VariableAndHash<std::string>{"human_node" + node_suffix};
            let[VehicleParameters::suffix] = node_suffix;
            return;
        case RemoteSceneObjectType::REMOTE_USERS:
        case RemoteSceneObjectType::PLAYER:
        case RemoteSceneObjectType::COUNTDOWN:
            throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
        }
        throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
    }();
    physics_scene.scene_.add_root_node(
        node_name,
        std::move(node),
        RenderingDynamics::MOVING,
        RenderingStrategies::OBJECT);
    [&](){
        switch (type) {
        case RemoteSceneObjectType::RIGID_BODY_CAR:
            initial[CarParameters::velocity] = v_com;
            initial[CarParameters::angular_velocity] = w;
            // CreateGenericCar(physics_scene).execute(initial);
            {
                auto command = nlohmann::json{
                    {"without", {"asset_id"}},
                    {"playback", "remote.vehicle.create_$asset_id"}
                };
                physics_scene
                .macro_line_executor_
                .inserted_block_arguments(initial)(command, nullptr);
            }
            return;
        case RemoteSceneObjectType::RIGID_BODY_AVATAR:
            initial[AvatarParameters::velocity] = v_com;
            initial[AvatarParameters::angular_velocity] = w;
            CreateGenericAvatar(
                physics_scene,
                physics_scene
                .macro_line_executor_
                .inserted_block_arguments(std::move(let))).execute(initial_view);
            return;
        case RemoteSceneObjectType::REMOTE_USERS:
        case RemoteSceneObjectType::PLAYER:
        case RemoteSceneObjectType::COUNTDOWN:
            throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
        }
        throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
    }();
    auto rb = get_rigid_body_vehicle(pnode.get(), CURRENT_SOURCE_LOCATION);
    rb->rbp_.v_com_ = v_com;
    rb->rbp_.w_ = w;
    rb->flags_ = flags;
    rb->remote_object_id_ = remote_object_id;
    if (owner_site_id.has_value()) {
        rb->owner_site_id_ = *owner_site_id;
    }
    return {
        global_object_pool.create<RemoteRigidBodyVehicle>(
            CURRENT_SOURCE_LOCATION,
            verbosity,
            type,
            std::move(initial),
            std::move(node_suffix),
            rb.set_loc(CURRENT_SOURCE_LOCATION),
            DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION}),
        CURRENT_SOURCE_LOCATION};
}

std::string RemoteRigidBodyVehicle::name() const {
    return rb_->name();
}

int32_t RemoteRigidBodyVehicle::priority() const {
    return RemoteSceneObjectPriority::RIGID_BODY_VEHICLE;
}

void RemoteRigidBodyVehicle::read(
    BinaryBitwiseWordsReader& reader,
    RemoteSiteId sender_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    TransmissionHistoryReader& transmission_history_reader)
{
    if (rb_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle::read: Rigid body is destroyed");
    }
    if (rb_->scene_node_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle::read: Scene node is destroyed");
    }
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != type_) {
        throw std::runtime_error("RemoteRigidBodyVehicle::read: Unexpected scene object type");
    }
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        [&](){
            switch (type_) {
            case RemoteSceneObjectType::RIGID_BODY_CAR:
                reader.read_string<StringLengthType>("asset_id");
                reader.read_string<StringLengthType>("suffix");
                reader.read_bool_bit("if_car_body_renderable_style");
                reader.read_bool_bit("if_damageable");
                reader.read_bool_bit("parking_brake_pulled");
                reader.read_binary<EFixedArray<float, 3>>("color");
                reader.read_binary<float>("velocity_error_std");
                reader.read_binary<float>("error_alpha");
                reader.read_binary<float>("yaw_error_std");
                reader.read_binary<float>("pitch_error_std");
                return;
            case RemoteSceneObjectType::RIGID_BODY_AVATAR:
                reader.read_string<StringLengthType>("asset_id");
                reader.read_string<StringLengthType>("suffix");
                reader.read_bool_bit("if_human_style");
                reader.read_bool_bit("if_damageable");
                reader.read_bool_bit("parking_brake_pulled");
                reader.read_binary<EFixedArray<float, 3>>("color");
                reader.read_binary<float>("velocity_error_std");
                reader.read_binary<float>("error_alpha");
                reader.read_binary<float>("locked_on_angle");
                reader.read_binary<float>("yaw_error_std");
                reader.read_binary<float>("pitch_error_std");
                reader.read_binary<float>("pitch_min");
                reader.read_binary<float>("pitch_max");
                reader.read_binary<float>("dpitch_max");
                reader.read_binary<float>("dyaw_max");
                reader.read_binary<float>("steering_multiplier");
                reader.read_string<StringLengthType>("animation_resource_wo_gun");
                reader.read_string<StringLengthType>("animation_resource_w_gun");
                reader.read_binary<float>("y_fov");
                reader.read_binary<float>("near_plane");
                reader.read_binary<float>("far_plane");
                reader.read_binary<bool>("with_gun");
                return;
            case RemoteSceneObjectType::REMOTE_USERS:
            case RemoteSceneObjectType::PLAYER:
            case RemoteSceneObjectType::COUNTDOWN:
                throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
            }
            throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
        }();
    }
    auto position = deserialize_position(reader, "position");
    auto v_com = deserialize_direction(reader, "v_com");
    FixedArray<SceneDir, 3> rotation = uninitialized;
    FixedArray<SceneDir, 3> w = uninitialized;
    [&](){
        switch (type) {
        case RemoteSceneObjectType::RIGID_BODY_CAR:
            rotation = deserialize_angles(reader, "rotation");
            w = deserialize_direction(reader, "w");
            return;
        case RemoteSceneObjectType::RIGID_BODY_AVATAR:
            rotation = {0.f, deserialize_angle(reader, "avatar yaw"), 0.f};
            w = {0.f, reader.read_binary<SceneDir>("w"), 0.f};
            return;
        case RemoteSceneObjectType::REMOTE_USERS:
        case RemoteSceneObjectType::PLAYER:
        case RemoteSceneObjectType::COUNTDOWN:
            throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
        }
        throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
    }();
    auto flags = reader.read_bits<RigidBodyVehicleFlags>(RIGID_BODY_VEHICLE_FLAGS_NBITS, "rigid body flags");
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        rb_->owner_site_id_ = reader.read_binary<RemoteSiteId>("owner_site_id");
    }
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted scene object type");
        if (end != ~type_) {
            throw std::runtime_error("Invalid rigid body vehicle end (1)");
        }
    }
    if (physics_scene_->remote_scene_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle: Remote scene is null");
    }
    if (!rb_->owner_site_id_.has_value()) {
        throw std::runtime_error("RemoteRigidBodyVehicle: Owner site ID not set");
    }
    auto privileges = RemotePrivileges{
        physics_scene_->remote_scene_->local_site_id(),
        sender_site_id,
        *rb_->owner_site_id_,
        remote_object_id.site_id};
    auto pf = PositionFlags::NONE;
    if (sum(squared(rb_->rbp_.abs_position() - position)) > squared(REMOTE_INTERPOLATION_JUMP_DISTANCE)) {
        pf |= PositionFlags::POSITION_CONTAINS_JUMP;
    }
    if (!privileges.is_manager_local) {
        if (rb_->is_deactivated_avatar() && !any(flags & RigidBodyVehicleFlags::IS_DEACTIVATED_AVATAR)) {
            pf |= PositionFlags::IS_REMOTELY_ACTIVATED_AVATAR;
        }
    }
    auto pp = privileges.position(pf);
    if (pp.invalidate_transformation_history) {
        rb_->scene_node_->set_absolute_pose(
            position,
            rotation,
            1.f,
            SceneTime::initial(physics_scene_->dynamic_world_.get_time()));
    }
    if (pp.update_position) {
        rb_->rbp_.set_pose(tait_bryan_angles_2_matrix(rotation), position);
        rb_->rbp_.v_com_ = v_com;
        rb_->rbp_.w_ = w;
    }
    if (!privileges.is_manager_local) {
        rb_->flags_ = flags;
    }
}

void RemoteRigidBodyVehicle::write(
    BinaryBitwiseWordsWriter& writer,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    TransmissionHistoryWriter& transmission_history_writer)
{
    if (rb_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle::write: Rigid body is destroyed");
    }
    auto transmitted_fields = TransmittedFields::NONE;
    transmitted_fields |= RigidBodyTransmittedFields::NONZERO;
    if (known_fields == KnownFields::NONE) {
        transmitted_fields |= RigidBodyTransmittedFields::INITIAL;
    }
    if (any(proxy_tasks & ProxyTasks::SEND_OWNERSHIP)) {
        transmitted_fields |= RigidBodyTransmittedFields::OWNERSHIP;
    }
    transmission_history_writer.write_remote_object_id(writer, remote_object_id, transmitted_fields);
    writer.write_binary(type_, "rigid body vehicle");
    if (any(transmitted_fields & RigidBodyTransmittedFields::INITIAL)) {
        JsonView jv{initial_};
        [&](){
            switch (type_) {
            case RemoteSceneObjectType::RIGID_BODY_CAR:
                writer.write_string<StringLengthType>(jv.at<std::string>(CarParameters::asset_id), "asset_id");
                writer.write_string<StringLengthType>(jv.at<std::string>(CarParameters::suffix), "suffix");
                writer.write_bool_bit(jv.at<bool>(CarParameters::if_car_body_renderable_style), "if_car_body_renderable_style");
                writer.write_bool_bit(jv.at<bool>(CarParameters::if_damageable), "if_damageable");
                writer.write_bool_bit(jv.at<bool>(CarParameters::parking_brake_pulled), "parking_brake_pulled");
                writer.write_binary(jv.at<EFixedArray<float, 3>>(CarParameters::color), "color");
                writer.write_binary(jv.at<float>(CarParameters::velocity_error_std), "velocity_error_std");
                writer.write_binary(jv.at<float>(CarParameters::error_alpha), "error_alpha");
                writer.write_binary(jv.at<float>(CarParameters::yaw_error_std), "yaw_error_std");
                writer.write_binary(jv.at<float>(CarParameters::pitch_error_std), "pitch_error_std");
                return;
            case RemoteSceneObjectType::RIGID_BODY_AVATAR:
                writer.write_string<StringLengthType>(jv.at<std::string>(AvatarParameters::asset_id), "asset_id");
                writer.write_string<StringLengthType>(jv.at<std::string>(AvatarParameters::suffix), "suffix");
                writer.write_bool_bit(jv.at<bool>(AvatarParameters::if_human_style), "if_car_body_renderable_style");
                writer.write_bool_bit(jv.at<bool>(AvatarParameters::if_damageable), "if_damageable");
                writer.write_bool_bit(jv.at<bool>(AvatarParameters::parking_brake_pulled), "parking_brake_pulled");
                writer.write_binary(jv.at<EFixedArray<float, 3>>(AvatarParameters::color), "color");
                writer.write_binary(jv.at<float>(AvatarParameters::velocity_error_std), "velocity_error_std");
                writer.write_binary(jv.at<float>(AvatarParameters::error_alpha), "error_alpha");
                writer.write_binary(jv.at<float>(AvatarParameters::locked_on_angle), "locked_on_angle");
                writer.write_binary(jv.at<float>(AvatarParameters::yaw_error_std), "yaw_error_std");
                writer.write_binary(jv.at<float>(AvatarParameters::pitch_error_std), "pitch_error_std");
                writer.write_binary(jv.at<float>(AvatarParameters::pitch_min), "pitch_min");
                writer.write_binary(jv.at<float>(AvatarParameters::pitch_max), "pitch_max");
                writer.write_binary(jv.at<float>(AvatarParameters::dpitch_max), "dpitch_max");
                writer.write_binary(jv.at<float>(AvatarParameters::dyaw_max), "dyaw_max");
                writer.write_binary(jv.at<float>(AvatarParameters::steering_multiplier), "steering_multiplier");
                writer.write_string<StringLengthType>(jv.at<std::string>(AvatarParameters::animation_resource_wo_gun), "animation_resource_wo_gun");
                writer.write_string<StringLengthType>(jv.at<std::string>(AvatarParameters::animation_resource_w_gun), "animation_resource_w_gun");
                writer.write_binary(jv.at<float>(AvatarParameters::y_fov), "y_fov");
                writer.write_binary(jv.at<float>(AvatarParameters::near_plane), "near_plane");
                writer.write_binary(jv.at<float>(AvatarParameters::far_plane), "far_plane");
                writer.write_binary(jv.at<bool>(AvatarParameters::with_gun), "with_gun");
                return;
            case RemoteSceneObjectType::REMOTE_USERS:
            case RemoteSceneObjectType::PLAYER:
            case RemoteSceneObjectType::COUNTDOWN:
                throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
            }
            throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
        }();
    }
    serialize_position(writer, rb_->rbp_.abs_position(), "position");
    serialize_direction(writer, rb_->rbp_.v_com_, "v_com");
    [&](){
        switch (type_) {
        case RemoteSceneObjectType::RIGID_BODY_CAR:
            serialize_angles(writer, matrix_2_tait_bryan_angles(rb_->rbp_.rotation_), "rotation");
            serialize_direction(writer, rb_->rbp_.w_, "w");
            return;
        case RemoteSceneObjectType::RIGID_BODY_AVATAR:
            serialize_angle(writer, z_to_yaw(rb_->rbp_.rotation_.column(2)), "avatar yaw");
            writer.write_binary<SceneDir>(rb_->rbp_.w_(1), "w");
            return;
        case RemoteSceneObjectType::REMOTE_USERS:
        case RemoteSceneObjectType::PLAYER:
        case RemoteSceneObjectType::COUNTDOWN:
            throw std::runtime_error("RemoteRigidBodyVehicle: Unexpected object type");
        }
        throw std::runtime_error("RemoteRigidBodyVehicle: Unknown scene object type");
    }();
    writer.write_bits(rb_->flags_, RIGID_BODY_VEHICLE_FLAGS_NBITS, "rigid body flags");
    if (any(transmitted_fields & RigidBodyTransmittedFields::OWNERSHIP)) {
        if (!rb_->owner_site_id_.has_value()) {
            throw std::runtime_error("Owner site ID not set");
        }
        writer.write_binary(*rb_->owner_site_id_, "owner site ID");
    }
    if (remote_end_check_enabled()) {
        writer.write_binary(~type_, "inverted rigid body vehicle");
    }
    writer.flush_partial("flush rigid body vehicle");
}

DanglingBaseClassRef<RigidBodyVehicle> RemoteRigidBodyVehicle::rb() {
    if (rb_ == nullptr) {
        throw std::runtime_error("RemoteRigidBodyVehicle::rb: Rigid body is destroyed");
    }
    return *rb_;
}

const std::string& RemoteRigidBodyVehicle::node_suffix() const {
    return node_suffix_;
}
