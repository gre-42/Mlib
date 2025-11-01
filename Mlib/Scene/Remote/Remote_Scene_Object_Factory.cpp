#include "Remote_Scene_Object_Factory.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Scene/Remote/Remote_Rigid_Body_Vehicle.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

RemoteSceneObjectFactory::RemoteSceneObjectFactory(
    const DanglingBaseClassRef<ObjectPool>& object_pool,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        IoVerbosity verbosity)
    : object_pool_{ object_pool }
    , physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
{}

RemoteSceneObjectFactory::~RemoteSceneObjectFactory() = default;

DanglingBaseClassPtr<IIncrementalObject> RemoteSceneObjectFactory::try_create_shared_object(std::istream& istr)
{
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    switch (type) {
    case RemoteSceneObjectType::RIGID_BODY_VEHICLE:
        {
            auto rb = RemoteRigidBodyVehicle::try_create_from_stream(object_pool_.get(), physics_scene_.get(), istr, verbosity_);
            if (rb == nullptr) {
                return nullptr;
            }
            return {
                object_pool_->add<RemoteRigidBodyVehicle>(std::move(rb), CURRENT_SOURCE_LOCATION), CURRENT_SOURCE_LOCATION};
        }
    }
    THROW_OR_ABORT("Unknown object type: " + std::to_string((int)type));
}
