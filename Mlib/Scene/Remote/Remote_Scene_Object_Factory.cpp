#include "Remote_Scene_Object_Factory.hpp"
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Scene/Remote/Remote_Countdown.hpp>
#include <Mlib/Scene/Remote/Remote_Player.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene/Remote/Remote_Users.hpp>

using namespace Mlib;

RemoteSceneObjectFactory::RemoteSceneObjectFactory(
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
    const DanglingBaseClassRef<SceneLevelSelector>& scene_level_selector,
    IoVerbosity verbosity)
    : physics_scene_{ physics_scene }
    , scene_level_selector_{ scene_level_selector }
    , verbosity_{ verbosity }
{}

RemoteSceneObjectFactory::~RemoteSceneObjectFactory() {
    on_destroy.clear();
}

DanglingBaseClassPtr<IIncrementalObject> RemoteSceneObjectFactory::try_create_shared_object(
    BinaryBitwiseWordsReader& reader,
    RemoteSiteId sender_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    ObjectLifetimeStatus lifetime_status,
    ProxyObjectsCaches& proxy_objects_caches,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    switch (type) {
    case RemoteSceneObjectType::REMOTE_USERS:
        if (lifetime_status == ObjectLifetimeStatus::DELETED) {
            throw std::runtime_error("REMOTE_USERS: Reading deleted objects not supported");
        }
        return RemoteUsers::try_create_from_stream(
            physics_scene_.get(), scene_level_selector_.get(), reader,
            transmitted_fields, remote_object_id.site_id,
            proxy_tasks, transmission_history_reader, verbosity_);
    case RemoteSceneObjectType::PLAYER:
        if (lifetime_status == ObjectLifetimeStatus::DELETED) {
            throw std::runtime_error("PLAYER: Reading deleted objects not supported");
        }
        return RemotePlayer::try_create_from_stream(
            physics_scene_.get(), reader, transmitted_fields,
            transmission_history_reader, verbosity_);
    case RemoteSceneObjectType::RIGID_BODY_CAR:
    case RemoteSceneObjectType::RIGID_BODY_AVATAR:
        return RemoteRigidBodyVehicle::try_create_from_stream(
            type, physics_scene_.get(), reader, sender_site_id,
            transmitted_fields, lifetime_status, remote_object_id, proxy_objects_caches, verbosity_);
    case RemoteSceneObjectType::COUNTDOWN:
        if (lifetime_status == ObjectLifetimeStatus::DELETED) {
            throw std::runtime_error("COUNTDOWN: Reading deleted objects not supported");
        }
        return RemoteCountdown::try_create_from_stream(
            physics_scene_.get(), reader,
            remote_object_id, verbosity_);
    }
    throw std::runtime_error("Unknown object type: " + std::to_string((int)type));
}
