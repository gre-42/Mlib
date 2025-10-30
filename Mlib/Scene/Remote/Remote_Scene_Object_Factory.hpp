#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>

namespace Mlib {

class ObjectPool;
class PhysicsScene;

class RemoteSceneObjectFactory: public IIncrementalObjectFactory {
public:
    explicit RemoteSceneObjectFactory(
        const DanglingBaseClassRef<ObjectPool>& object_pool,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    virtual ~RemoteSceneObjectFactory() override;
    virtual DanglingBaseClassRef<IIncrementalObject> create_shared_object(std::istream& istr) override;
private:
    DanglingBaseClassRef<ObjectPool> object_pool_;
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
};

}
