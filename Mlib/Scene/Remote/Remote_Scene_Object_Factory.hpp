#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>

namespace Mlib {

class PhysicsScene;
enum class IoVerbosity;

class RemoteSceneObjectFactory final: public IIncrementalObjectFactory {
public:
    explicit RemoteSceneObjectFactory(
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        IoVerbosity verbosity);
    virtual ~RemoteSceneObjectFactory() override;
    virtual DanglingBaseClassPtr<IIncrementalObject> try_create_shared_object(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override;
private:
    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
};

}
