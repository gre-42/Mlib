#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Math/Is_Newer.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Os/Io/Ostream_Size_Logger.hpp>
#include <Mlib/Os/Io/Serialize/Serialize.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Incremental_Versions.hpp>
#include <Mlib/Remote/Incremental_Objects/Known_Fields.hpp>
#include <Mlib/Remote/Incremental_Objects/Object_Lifetime_Status.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Scene_Level.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>
#include <Mlib/Remote/Session_Id.hpp>
#include <Mlib/Remote/Statistics/Remote_Statistics_Verbosity.hpp>
#include <chrono>
#include <compare>

using namespace Mlib;

IncrementalCommunicatorProxy::IncrementalCommunicatorProxy(
    std::shared_ptr<ISendSocket> send_socket,
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
    const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_objects_caches,
    const TransmissionLut& full_transmission_lut,
    IoVerbosity verbosity,
    ProxyTasks tasks,
    RemoteSiteId home_site_id)
    : incremental_cache_proxy_token_{ proxy_objects_caches, home_site_id }
    , receive_datagram_counter_{ 0 }
    , send_datagram_counter_{ 0 }
    , send_socket_{ std::move(send_socket) }
    , shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , proxy_objects_caches_{ proxy_objects_caches }
    , full_transmission_lut_{ full_transmission_lut }
    , verbosity_{ verbosity }
    , tasks_{ tasks }
    , home_site_id_{ home_site_id }
{
    if (any(tasks_ & ProxyTasks::SEND_OWNERSHIP)) {
        session_id_ = 0;
    } else {
        session_id_ = get_session_id(0);
    }
}

IncrementalCommunicatorProxy::~IncrementalCommunicatorProxy() {
    on_destroy.clear();
}

void IncrementalCommunicatorProxy::set_send_socket(std::shared_ptr<ISendSocket> send_socket) {
    send_socket_ = std::move(send_socket);
}

static bool is_loading(LocalSceneLevelLoadStatus status) {
    switch (status) {
    case LocalSceneLevelLoadStatus::LOADING:
        return true;
    case LocalSceneLevelLoadStatus::RUNNING:
        return false;
    }
    throw std::runtime_error("Unknown scene level load status");
}

void IncrementalCommunicatorProxy::receive_from_home(std::istream& istr) {
    std::optional<LocalSceneLevel> home_scene_level;
    auto reader = BinaryBitwiseWordsReader{istr, nullptr, verbosity_};
    auto session_id = reader.read_binary<SessionIdType>("session ID");
    if (any(tasks_ & ProxyTasks::SEND_OWNERSHIP)) {
        objects_->delete_orphaned_objects(home_site_id_, session_id);
    } else {
        auto nsites = reader.read_binary<NSitesType>("#sites");
        for (NSitesType i = 0; i < nsites; ++i) {
            auto site_id = reader.read_binary<RemoteSiteId>("remote site ID");
            auto site_session_id = reader.read_binary<SessionIdType>("#site session ID");
            if (site_id == home_site_id_) {
                throw std::runtime_error("Received home site ID");
            }
            if (site_id == objects_->local_site_id()) {
                throw std::runtime_error("Received local site ID");
            }
            objects_->delete_orphaned_objects(site_id, site_session_id);
        }
    }
    auto remote_time = reader.read_binary<RemoteTimeCount>("remote time [ms]");
    {
        auto scene_level_name = reader.read_string<StringLengthType>("scene level name");
        auto time_of_day = reader.read_string<StringLengthType>("time of day");
        auto reload_count = reader.read_binary<ReloadCountType>("reload_count");
        home_scene_level.emplace(std::move(scene_level_name), std::move(time_of_day), reload_count);
        auto level_selector = objects_->local_scene_level_selector();
        if (any(tasks_ & ProxyTasks::RELOAD_SCENE)) {
            if (level_selector->client_set_next_scene_level(
                home_scene_level->level_name,
                home_scene_level->time_of_day,
                home_scene_level->reload_count))
            {
                return;
            }
        } else if (level_selector->reload_required(*home_scene_level)) {
            return;
        }
        auto home_load_level_status = reader.read_binary<LocalSceneLevelLoadStatus>("scene level load status");
        if (is_loading(level_selector->load_status()) ||
            is_loading(home_load_level_status))
        {
            return;
        }
    }
    if (any(tasks_ & ProxyTasks::SEND_OWNERSHIP)) {
        // Server code
        if (session_id != session_id_) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Server received differing session ID. Server: " <<
                    (session_id_ + 0) <<
                    ", client: " << (session_id + 0);
            }
            session_id_ = session_id;
            socket_versions_ = {};
            proxy_objects_caches_->remove_proxy(home_site_id_);
        }
    } else {
        // Client code
        if (session_id != session_id_) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Client received differing session ID. Client: " <<
                    (session_id_+ 0) <<
                    ", server: " << (session_id + 0);
            }
            return;
        }
    }
    auto versions = reader.deserialize<IncrementalVersionsRead>("incremental versions");
    stats_.notify_datagram(versions.remote_new_version);
    if (get_print_transmission_stastics()) {
        if (receive_datagram_counter_ % (5 * 60) == 0) {
            linfo() << "Receive stats: " << stats_;
        }
    }
    ++receive_datagram_counter_;
    if (!is_older(socket_versions_.remote_version, versions.remote_new_version)) {
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Detected outdated or duplicate datagram. Stored: " << (socket_versions_.remote_version + 0) <<
                ", received: " << (versions.remote_base_version + 0);
        }
        return;
    }
    socket_versions_.local.remote_version = versions.local_remote_version;
    socket_versions_.remote_version = versions.remote_new_version;
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "receive versions " << versions;
    }

    std::unordered_set<RemoteObjectId> objects_known_by_home;
    {
        auto ndeleted = reader.read_binary<NDeletedType>("#deleted");
        for (NDeletedType i = 0; i < ndeleted; ++i) {
            auto id = reader.deserialize<RemoteObjectId>("deleted ID");
            if (objects_->try_remove(id)) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Delete " << id;
                }
            }
        }
    }
    {
        objects_unknown_at_home_ = {};
        auto nunknown = reader.read_binary<NUnknownType>("#unknown");
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << this << ' ' << (nunknown + 0) << " objects unknown to home site " << (home_site_id_ + 0);
        }
        for (NUnknownType i = 0; i < nunknown; ++i) {
            auto id = reader.deserialize<RemoteObjectId>("unknown ID");
            objects_unknown_at_home_.insert(id);
        }
    }
    {
        objects_unknown_here_ = {};
        auto transmission_history_reader = TransmissionHistoryReader{*home_scene_level, remote_time, objects_->local_time()};
        auto receive_any = [&](RemoteObjectVisibility visibility){
            const auto& deleted_objects_long = objects_->deleted_objects_long();
            // linfo() << "Received " << object_count << " objects_";
            while (true) {
                auto transmitted_fields = reader.read_binary<TransmittedFields>("transmitted fields");
                if (transmitted_fields == TransmittedFields::NONE) {
                    break;
                }
                auto i = transmission_history_reader.read_remote_object_id(reader, transmitted_fields);
                objects_known_by_home.insert(i);
                if (auto it = objects_->try_get(i); it != nullptr) {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << this << " read from home site " << (home_site_id_ + 0) << ", object " << i << " \"" << it->name() << '"';
                    }
                    it->read(reader, home_site_id_, i, tasks_, transmitted_fields,
                        proxy_objects_caches_.get(), versions, transmission_history_reader);
                } else {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << this << " create from home site " << (home_site_id_ + 0) << ", object " << i;
                    }
                    auto lifetime_status = deleted_objects_long.contains_key(i)
                        ? ObjectLifetimeStatus::DELETED
                        : ObjectLifetimeStatus::EXISTS;
                    auto o = shared_object_factory_->try_create_shared_object(
                        reader, home_site_id_, i, tasks_, transmitted_fields, lifetime_status,
                        proxy_objects_caches_.get(), versions, transmission_history_reader);
                    if (o == nullptr) {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << this << " cannot create object";
                        }
                        if (i.site_id != objects_->local_site_id()) {
                            objects_unknown_here_.insert(i);
                        }
                    } else {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << this << " object created: \"" << o->name() << '"';
                        }
                        objects_->add_remote_object(i, *o, visibility);
                    }
                }
            }
        };
        receive_any(RemoteObjectVisibility::PRIVATE);
        receive_any(RemoteObjectVisibility::PUBLIC);
        receive_any(RemoteObjectVisibility::PUBLIC);
    }
    auto delete_unknown_objects = [&](const RemoteObjects& objects){
        std::vector<RemoteObjectId> objects_to_be_deleted;
        objects_to_be_deleted.reserve(objects.size());
        for (auto& [i, _] : objects) {
            bool can_delete = [&](){
                if (any(tasks_ & ProxyTasks::SEND_OWNERSHIP)) {
                    return i.site_id == home_site_id_;
                } else {
                    return i.site_id != objects_->local_site_id();
                }
            }();
            if (can_delete && !objects_known_by_home.contains(i)) {
                objects_to_be_deleted.push_back(i);
            }
        }
        for (auto i : objects_to_be_deleted) {
            if (objects_->try_remove(i)) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Delete " << i;
                }
            }
        }
    };
    delete_unknown_objects(objects_->private_remote_objects());
    delete_unknown_objects(objects_->public_remote_objects());
}

struct MatchedAndPriority {
    bool matched;
    int32_t priority;
    FullRetransmissionAge age;
    std::strong_ordering operator <=> (const MatchedAndPriority&) const = default;
};

void IncrementalCommunicatorProxy::send_home(
    std::iostream& iostr,
    SendStatusCode& status_code)
{
    std::optional<OstreamSizeLogger> sl;
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        sl.emplace(iostr, "Send home [bytes]: ");
    }
    std::unordered_map<RemoteObjectId, uint32_t> full_retransmission_age;
    auto compute_full_transmission_age = [&](const RemoteObjectId& i, const IIncrementalObject& o){
        if (objects_unknown_at_home_.contains(i)) {
            full_retransmission_age.emplace(i, std::numeric_limits<FullRetransmissionAge>::max());
        } else {
            full_retransmission_age.emplace(i, o.full_retransmission_age(home_site_id_, proxy_objects_caches_.get()));
        }
    };
    std::optional<RemoteObjectId> object_to_send_completely;
    auto full_transmission_mask = full_transmission_lut_();
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "Full transmission mask: " << full_transmission_mask;
    }
    {
        std::optional<MatchedAndPriority> highest_priority;
        auto update_common = [&, compute_full_transmission_age](const RemoteObjectId& i, const IIncrementalObject& o){
            compute_full_transmission_age(i, o);
            auto age = full_retransmission_age.at(i);
            auto matched = bool(o.full_transmission_mask() & full_transmission_mask);
            auto op = MatchedAndPriority{matched, o.priority(), age};
            if (!object_to_send_completely.has_value() || (op > *highest_priority)) {
                object_to_send_completely.emplace(i);
                highest_priority.emplace(op);
            }
        };
        auto update_object_to_send_completely_local = [&](const LocalObjects& objects){
            for (const auto& [i, o] : objects) {
                update_common(RemoteObjectId{objects_->local_site_id(), i}, o.get());
            }
        };
        auto update_object_to_send_completely_remote = [&](const RemoteObjects& objects){
            for (const auto& [i, o] : objects) {
                update_common(i, o.get());
            }
        };
        update_object_to_send_completely_local(objects_->private_local_objects());
        update_object_to_send_completely_local(objects_->public_local_objects());
        update_object_to_send_completely_remote(objects_->public_remote_objects());
    }
    auto writer = BinaryBitwiseWordsWriter{iostr, nullptr};
    writer.write_binary(session_id_, "session ID");
    if (any(tasks_ & ProxyTasks::SEND_OWNERSHIP)) {
        const auto& session_ids = objects_->session_ids();
        writer.write_binary(
            integral_cast<NSitesType>(session_ids.size() - session_ids.contains(home_site_id_)),
            "#sites");
        for (const auto& [site_id, session_id] : session_ids) {
            if ((site_id != home_site_id_) &&
                (site_id != objects_->local_site_id()))
            {
                writer.write_binary(site_id, "site ID");
                writer.write_binary(session_id, "session ID");
            }
        }
    }
    writer.write_binary(
        std::chrono::duration_cast<std::chrono::duration<RemoteTimeCount, RemoteTimeRatio>>(
            objects_->local_time().time_since_epoch()).count(),
            "remote time [ms]");
    switch (0) { case 0:
        {
            auto level_selector = objects_->local_scene_level_selector();
            auto level = level_selector->get_local_scene_level();
            writer.write_string<StringLengthType>(level.level_name, "level name");
            writer.write_string<StringLengthType>(level.time_of_day, "time of day");
            writer.write_binary(level.reload_count, "reload count");
            writer.write_binary(level_selector->load_status(), "scene level load status");
            if (is_loading(level_selector->load_status())) {
                break;
            }
        }
        ++socket_versions_.local.local_version;
        socket_versions_.local.local_version = std::max(DatagramIndexType(1), socket_versions_.local.local_version);
        auto versions = IncrementalVersionsWrite{
            .remote_local_version = socket_versions_.remote_version,        // local_remote_version
            .local_base_version = socket_versions_.local.remote_version,    // remote_base_version
            .local_new_version = socket_versions_.local.local_version       // remote_new_version
        };
        writer.serialize(versions, "incremental versions");
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "send versions " << versions;
        }
        {
            std::optional<OstreamSizeLogger> sl;
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                sl.emplace(iostr, "Deleted objects [bytes]: ");
            }
            const auto& deleted_short = objects_->deleted_objects_short();
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Delete " << deleted_short.size() << " objects (short)";
            }
            writer.write_binary(integral_cast<NDeletedType>(deleted_short.size()), "#ndeleted");
            for (const auto& [id, time] : deleted_short) {
                writer.serialize(id, "deleted ID (short)");
            }
        }
        {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << objects_unknown_here_.size() << " objects unknown";
            }
            writer.write_binary(integral_cast<NUnknownType>(objects_unknown_here_.size()), "#unknown");
            for (const auto& id : objects_unknown_here_) {
                writer.serialize(id, "unknown ID");
            }
        }
        {
            bool new_object_sent = false;
            auto transmission_history_writer = TransmissionHistoryWriter{objects_->local_time(), send_datagram_counter_};
            auto send_object = [&](RemoteObjectId i, const DestructionFunctionsTokensRef<IIncrementalObject>& o){
                auto known_fields = (full_retransmission_age.at(i) != 0)
                    ? KnownFields::NONE
                    : KnownFields::ALL;
                if (known_fields == KnownFields::NONE) {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Maybe send complete object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                    if (!object_to_send_completely.has_value()) {
                        throw std::runtime_error((std::stringstream() << "Object to send completely not set: " << i).str());
                    }
                    if (new_object_sent || (i != *object_to_send_completely)) {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << "Send object only partially";
                        }
                        known_fields = KnownFields::ALL;
                    } else {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << "Send complete object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                        }
                        new_object_sent = true;
                    }
                } else {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Send partial object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                }
                std::optional<OstreamSizeLogger> sl;
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    sl.emplace(iostr, o->name() + " [bytes]: ");
                }
                o->write(writer, home_site_id_, i, tasks_, known_fields, proxy_objects_caches_.get(), versions, transmission_history_writer);
            };
            auto send_local = [&](const LocalObjects& objects){
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Maybe send " << objects.size() << " local objects";
                }
                for (auto& [i, o] : objects) {
                    auto j = RemoteObjectId{objects_->local_site_id(), i};
                    send_object(j, o);
                }
                writer.write_binary(TransmittedFields::NONE, "transmitted fields EOF");
            };
            auto send_zero = [&](const char* msg){
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Send no " << msg << " objects";
                }
                writer.write_binary(TransmittedFields::NONE, "transmitted fields EOF");
            };
            if (any(tasks_ & ProxyTasks::SEND_LOCAL)) {
                send_local(objects_->private_local_objects());
                send_local(objects_->public_local_objects());
            } else {
                send_zero("local");
                send_zero("local");
            }
            if (any(tasks_ & ProxyTasks::SEND_REMOTE)) {
                const auto& objects = objects_->public_remote_objects();
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Maybe send " << objects.size() << " remote objects";
                }
                for (auto& [i, o] : objects) {
                    send_object(i, o);
                }
                writer.write_binary(TransmittedFields::NONE, "transmitted fields EOF");
            } else {
                send_zero("remote");
            }
        }
    }
    writer.flush_partial("before send");
    send_socket_->send(iostr, status_code);
    ++send_datagram_counter_;
}
