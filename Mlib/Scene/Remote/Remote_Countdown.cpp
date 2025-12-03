#include "Remote_Countdown.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

static_assert(sizeof(FixedArray<float, 3>) == 3 * 4);

RemoteCountdown::RemoteCountdown(
    IoVerbosity verbosity,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
    , physics_scene_on_destroy_{ physics_scene->on_destroy, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteCountdown";
    }
    physics_scene_on_destroy_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteCountdown::~RemoteCountdown() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteCountdown";
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemoteCountdown> RemoteCountdown::try_create_from_stream(
    PhysicsScene& physics_scene,
    std::istream& istr,
    const RemoteObjectId& remote_object_id,
    IoVerbosity verbosity)
{
    auto res = global_object_pool.create_unique<RemoteCountdown>(
        CURRENT_SOURCE_LOCATION,
        verbosity,
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION});
    res->read_data(istr, remote_object_id);
    return {res.release(), CURRENT_SOURCE_LOCATION};
}

void RemoteCountdown::read(
    std::istream& istr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = read_binary<RemoteSceneObjectType>(istr, "scene object type", verbosity_);
    if (type != RemoteSceneObjectType::COUNTDOWN) {
        THROW_OR_ABORT("RemoteCountdown::read: Unexpected scene object type");
    }
    read_data(istr, remote_object_id);
}

void RemoteCountdown::read_data(std::istream& istr, const RemoteObjectId& remote_object_id) {
    auto reader = BinaryReader(istr, verbosity_);
    float elapsed = reader.read_binary<float>("elapsed");
    float duration = reader.read_binary<float>("duration");
    if (!physics_scene_->remote_sites_->get_local_site_id().has_value()) {
        THROW_OR_ABORT("Local site ID not set");
    }
    if (remote_object_id.site_id != *physics_scene_->remote_sites_->get_local_site_id()) {
        physics_scene_->countdown_start_.set(elapsed, duration);
    }
    auto end = reader.read_binary<uint32_t>("inverted countdown");
    if (end != ~(uint32_t)RemoteSceneObjectType::COUNTDOWN) {
        THROW_OR_ABORT("Invalid countdown end");
    }
}

void RemoteCountdown::write(
    std::ostream& ostr,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    TransmissionHistoryWriter& transmission_history_writer)
{
    if (!physics_scene_->remote_sites_->get_local_site_id().has_value()) {
        THROW_OR_ABORT("Local site ID not set");
    }
    if (remote_object_id.site_id != *physics_scene_->remote_sites_->get_local_site_id()) {
        return;
    }
    transmission_history_writer.write_remote_object_id(ostr, remote_object_id, TransmittedFields::END);
    
    auto writer = BinaryWriter{ostr};
    writer.write_binary(RemoteSceneObjectType::COUNTDOWN, "countdown");
    writer.write_binary(physics_scene_->countdown_start_.elapsed(), "elapsed");
    writer.write_binary(physics_scene_->countdown_start_.duration(), "duration");
    writer.write_binary(~(uint32_t)RemoteSceneObjectType::COUNTDOWN, "inverted countdown");
}
