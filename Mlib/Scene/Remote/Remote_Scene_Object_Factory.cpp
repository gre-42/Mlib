#include "Remote_Scene_Object_Factory.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
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
    std::istream& istr,
    const RemoteObjectId& remote_object_id,
    TransmittedFields transmitted_fields,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    switch (type) {
    case RemoteSceneObjectType::REMOTE_USERS:
        return RemoteUsers::try_create_from_stream(physics_scene_.get(), scene_level_selector_.get(), istr, remote_object_id.site_id, transmission_history_reader, verbosity_);
    case RemoteSceneObjectType::PLAYER:
        return RemotePlayer::try_create_from_stream(physics_scene_.get(), istr, transmitted_fields, transmission_history_reader, verbosity_);
    case RemoteSceneObjectType::RIGID_BODY_CAR:
    case RemoteSceneObjectType::RIGID_BODY_AVATAR:
        return RemoteRigidBodyVehicle::try_create_from_stream(type, physics_scene_.get(), istr, transmitted_fields, remote_object_id, verbosity_);
    case RemoteSceneObjectType::COUNTDOWN:
        return RemoteCountdown::try_create_from_stream(physics_scene_.get(), istr, remote_object_id, verbosity_);
    }
    THROW_OR_ABORT("Unknown object type: " + std::to_string((int)type));
}
