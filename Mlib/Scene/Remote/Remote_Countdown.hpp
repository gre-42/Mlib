#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <memory>

namespace Mlib {

class PhysicsScene;
enum class IoVerbosity;

class RemoteCountdown final: public IIncrementalObject {
public:
    explicit RemoteCountdown(
        IoVerbosity verbosity,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene);
    ~RemoteCountdown();
    static DanglingBaseClassPtr<RemoteCountdown> try_create_from_stream(
        PhysicsScene& physics_scene,
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        IoVerbosity verbosity);
    virtual void read(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override;

private:
    void read_data(std::istream& istr, const RemoteObjectId& remote_object_id);

    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    IoVerbosity verbosity_;
    DestructionFunctionsRemovalTokens physics_scene_on_destroy_;
};

}
