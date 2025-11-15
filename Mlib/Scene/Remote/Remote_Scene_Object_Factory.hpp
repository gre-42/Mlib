#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>

namespace Mlib {

class ObjectPool;
class PhysicsScene;
enum class IoVerbosity;

class RemoteSceneObjectFactory final: public IIncrementalObjectFactory {
public:
    explicit RemoteSceneObjectFactory(
        const DanglingBaseClassRef<ObjectPool>& object_pool,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        IoVerbosity verbosity);
    virtual ~RemoteSceneObjectFactory() override;
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        std::istream& istr,
        TransmittedFields transmitted_fields,
        const RemoteObjectId& id) override;
private:
    DanglingBaseClassRef<ObjectPool> object_pool_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
};

}
