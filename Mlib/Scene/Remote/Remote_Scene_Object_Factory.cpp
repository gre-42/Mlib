#include "Remote_Scene_Object_Factory.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/Remote_Object_Id.hpp>
#include <Mlib/Scene/Remote/Remote_Player.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>
#include <Mlib/Scene/Remote/Remote_Users.hpp>

using namespace Mlib;

RemoteSceneObjectFactory::RemoteSceneObjectFactory(
    const DanglingBaseClassRef<ObjectPool>& object_pool,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        IoVerbosity verbosity)
    : object_pool_{ object_pool }
    , physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
{}

RemoteSceneObjectFactory::~RemoteSceneObjectFactory() {
    on_destroy.clear();
}

DanglingBaseClassPtr<IIncrementalObject> RemoteSceneObjectFactory::try_create_shared_object(
    std::istream& istr,
    TransmittedFields transmitted_fields,
    const RemoteObjectId& id)
{
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    switch (type) {
    case RemoteSceneObjectType::REMOTE_USERS:
        return RemoteUsers::try_create_from_stream(object_pool_.get(), physics_scene_.get(), istr, id.site_id, verbosity_);
    case RemoteSceneObjectType::PLAYER:
        return RemotePlayer::try_create_from_stream(object_pool_.get(), physics_scene_.get(), istr, transmitted_fields, verbosity_);
    case RemoteSceneObjectType::RIGID_BODY_VEHICLE:
        return RemoteRigidBodyVehicle::try_create_from_stream(object_pool_.get(), physics_scene_.get(), istr, transmitted_fields, id, verbosity_);
    }
    THROW_OR_ABORT("Unknown object type: " + std::to_string((int)type));
}
