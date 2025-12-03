#pragma once
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <memory>

namespace Mlib {

class SceneLevelSelector;
class PhysicsScene;
enum class IoVerbosity;

class RemoteUsers final: public IIncrementalObject {
public:
    explicit RemoteUsers(
        IoVerbosity verbosity,
        const DanglingBaseClassRef<PhysicsScene>& physics_scene,
        const DanglingBaseClassRef<SceneLevelSelector>& local_scene_level_selector,
        RemoteSiteId site_id);
    ~RemoteUsers();
    static DanglingBaseClassPtr<RemoteUsers> try_create_from_stream(
        PhysicsScene& physics_scene,
        SceneLevelSelector& scene_level_selector,
        std::istream& istr,
        RemoteSiteId site_id,
        ProxyTasks proxy_tasks,
        TransmissionHistoryReader& transmission_history_reader,
        IoVerbosity verbosity);
    virtual void read(
        std::istream& istr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        TransmittedFields transmitted_fields,
        TransmissionHistoryReader& transmission_history_reader) override;
    virtual void write(
        std::ostream& ostr,
        const RemoteObjectId& remote_object_id,
        ProxyTasks proxy_tasks,
        KnownFields known_fields,
        TransmissionHistoryWriter& transmission_history_writer) override;

private:
    void read_data(
        std::istream& istr,
        ProxyTasks proxy_tasks,
        TransmissionHistoryReader& transmission_history_reader);

    DanglingBaseClassRef<PhysicsScene> physics_scene_;
    DanglingBaseClassRef<SceneLevelSelector> local_scene_level_selector_;
    IoVerbosity verbosity_;
    RemoteSiteId site_id_;
    DestructionFunctionsRemovalTokens physics_scene_on_destroy_;
};

}
