#include "Remote_Game_Statistics.hpp"
#include <Mlib/Math/Saturating_Increment.hpp>
#include <Mlib/Misc/To_Underlying.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Players/Containers/Remote_Sites.hpp>
#include <Mlib/Remote/Incremental_Objects/Known_Fields.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Remote_Check.hpp>
#include <Mlib/Scene/Physics_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Priority.hpp>
#include <Mlib/Scene/Remote/Remote_Scene_Object_Type.hpp>

using namespace Mlib;

enum class StatisticsTransmittedFields: TransmittedFieldsType {
    NONZERO = (TransmittedFieldsType)TransmittedFields::END,
    FRAGS = (TransmittedFieldsType)TransmittedFields::END << 1,
};

inline TransmittedFields operator & (TransmittedFields a, StatisticsTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a & (TransmittedFieldsType)b);
}

inline TransmittedFields operator | (TransmittedFields a, StatisticsTransmittedFields b) {
    return (TransmittedFields)((TransmittedFieldsType)a | (TransmittedFieldsType)b);
}

inline TransmittedFields& operator |= (TransmittedFields& a, StatisticsTransmittedFields b) {
    (TransmittedFieldsType&)a |= (TransmittedFieldsType)b;
    return a;
}

RemoteGameStatistics::RemoteGameStatistics(
    IoVerbosity verbosity,
    const DanglingBaseClassRef<PhysicsScene>& physics_scene)
    : physics_scene_{ physics_scene }
    , verbosity_{ verbosity }
    , full_retransmission_age_{ 0 }
    , physics_scene_on_destroy_{ physics_scene->on_destroy.deflt, CURRENT_SOURCE_LOCATION }
{
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Create RemoteGameStatistics";
    }
    physics_scene_on_destroy_.add([this](){ global_object_pool.remove(this); }, CURRENT_SOURCE_LOCATION);
}

RemoteGameStatistics::~RemoteGameStatistics() {
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Destroy RemoteGameStatistics";
    }
    on_destroy.clear();
}

DanglingBaseClassPtr<RemoteGameStatistics> RemoteGameStatistics::try_create_from_stream(
    PhysicsScene& physics_scene,
    BinaryBitwiseWordsReader& reader,
    TransmittedFields transmitted_fields,
    const RemoteObjectId& remote_object_id,
    IoVerbosity verbosity)
{
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        StatisticsTransmittedFields::NONZERO |
        StatisticsTransmittedFields::FRAGS)))
    {
        throw std::runtime_error("RemoteGameStatistics::try_create_from_stream: Unknown transmitted fields");
    }
    auto res = global_object_pool.create_unique<RemoteGameStatistics>(
        CURRENT_SOURCE_LOCATION,
        verbosity,
        DanglingBaseClassRef<PhysicsScene>{physics_scene, CURRENT_SOURCE_LOCATION});
    res->read_data(reader, remote_object_id, transmitted_fields);
    return {res.release(), CURRENT_SOURCE_LOCATION};
}

std::string RemoteGameStatistics::name() const {
    return "game statistics";
}

int32_t RemoteGameStatistics::priority() const {
    return RemoteSceneObjectPriority::GAME_STATISTICS;
}

uint32_t RemoteGameStatistics::full_transmission_mask() const {
    return FullTransmissionMask::GAME_STATISTICS;
}

uint32_t RemoteGameStatistics::full_retransmission_age(
    RemoteSiteId receiver_site_id,
    ProxyObjectsCaches& proxy_objects_caches) const
{
    return full_retransmission_age_;
}

void RemoteGameStatistics::read(
    BinaryBitwiseWordsReader& reader,
    RemoteSiteId sender_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    TransmittedFields transmitted_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    const IncrementalVersionsRead& versions,
    TransmissionHistoryReader& transmission_history_reader)
{
    auto type = reader.read_binary<RemoteSceneObjectType>("scene object type");
    if (type != RemoteSceneObjectType::GAME_STATISTICS) {
        throw std::runtime_error((std::stringstream() <<
            "RemoteGameStatistics::read: Unexpected scene object type. Object ID = " <<
            remote_object_id << ", type = 0x" << std::hex << (to_underlying(type) + 0)).str());
    }
    if (any(transmitted_fields & ~(
        TransmittedFields::SITE_ID |
        StatisticsTransmittedFields::NONZERO |
        StatisticsTransmittedFields::FRAGS)))
    {
        throw std::runtime_error("RemoteGameStatistics::read: Unknown transmitted fields");
    }
    read_data(reader, remote_object_id, transmitted_fields);
}

void RemoteGameStatistics::read_data(
    BinaryBitwiseWordsReader& reader,
    const RemoteObjectId& remote_object_id,
    TransmittedFields transmitted_fields)
{
    if (any(transmitted_fields & StatisticsTransmittedFields::FRAGS)) {
        physics_scene_->players_.statistics.forget_oldest_kill_events(MAX_FRAG_EVENTS);
        auto nfrags = reader.read_binary<NFragEventsType>("#frags");
        for (size_t i = 0; i < nfrags; ++i) {
            KillEvent e;
            load(reader, e, "Frag event");
            physics_scene_->players_.statistics.notify_remote_kill(std::move(e));
        }
    }
    if (remote_end_check_enabled()) {
        auto end = reader.read_binary<RemoteSceneObjectType>("inverted game statistics");
        if (end != ~RemoteSceneObjectType::GAME_STATISTICS) {
            throw std::runtime_error("Invalid game statistics end");
        }
    }
}

void RemoteGameStatistics::write(
    BinaryBitwiseWordsWriter& writer,
    RemoteSiteId receiver_site_id,
    const RemoteObjectId& remote_object_id,
    ProxyTasks proxy_tasks,
    KnownFields known_fields,
    ProxyObjectsCaches& proxy_objects_caches,
    const IncrementalVersionsWrite& versions,
    TransmissionHistoryWriter& transmission_history_writer)
{
    auto transmitted_fields = TransmittedFields::NONE;
    transmitted_fields |= StatisticsTransmittedFields::NONZERO;
    if (known_fields == KnownFields::NONE) {
        transmitted_fields |= StatisticsTransmittedFields::FRAGS;
    }
    transmission_history_writer.write_remote_object_id(writer, remote_object_id, transmitted_fields);
    writer.write_binary(RemoteSceneObjectType::GAME_STATISTICS, "game statistics");
    if (any(transmitted_fields & StatisticsTransmittedFields::FRAGS)) {
        physics_scene_->players_.statistics.forget_oldest_kill_events(MAX_FRAG_EVENTS);
        const auto& events = physics_scene_->players_.statistics.newest_kill_events();
        writer.write_binary(integral_cast<NFragEventsType>(events.size()), "#frags");
        for (const auto& e : events) {
            save(writer, *e, "Frag");
        }
        full_retransmission_age_ = 0;
    } else {
        full_retransmission_age_ = saturating_increment(full_retransmission_age_);
    }
    if (remote_end_check_enabled()) {
        writer.write_binary(~RemoteSceneObjectType::GAME_STATISTICS, "inverted statistics");
    }
}
