#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Os/Io/Binary.hpp>
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
#include <chrono>
#include <concepts>

using namespace Mlib;

SessionIdType get_session_id() {
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
    return std::max(SessionIdType(1), SessionIdType(duration.count()));
}

IncrementalCommunicatorProxy::IncrementalCommunicatorProxy(
    std::shared_ptr<ISendSocket> send_socket,
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
    const DanglingBaseClassRef<ProxyObjectsCaches>& proxy_objects_caches,
    IoVerbosity verbosity,
    ProxyTasks tasks,
    RemoteSiteId home_site_id)
    : incremental_cache_proxy_token_{ proxy_objects_caches, home_site_id }
    , datagram_counter_{ 0 }
    , send_socket_{ std::move(send_socket) }
    , shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , proxy_objects_caches_{ proxy_objects_caches }
    , verbosity_{ verbosity }
    , tasks_{ tasks }
    , home_site_id_{ home_site_id }
{
    if (any(tasks_ & ProxyTasks::RELOAD_SCENE)) {
        session_id_ = get_session_id();
    } else {
        session_id_ = 0;
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

template <std::unsigned_integral T>
static bool is_newer(T incoming, T newest) {
    // Deduce the signed counterpart (e.g., uint16_t -> int16_t)
    using SignedType = std::make_signed_t<T>;
    return SignedType(incoming - newest) > 0;
}

void IncrementalCommunicatorProxy::receive_from_home(std::istream& istr) {
    std::optional<LocalSceneLevel> home_scene_level;
    auto reader = BinaryBitwiseWordsReader{istr, nullptr, verbosity_};
    auto session_id = reader.read_binary<SessionIdType>("session ID");
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
    if (any(tasks_ & ProxyTasks::RELOAD_SCENE)) {
        // Client code
        if (session_id != session_id_) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Client received differing session ID. Client: " <<
                    (session_id_+ 0) <<
                    ", server: " << (session_id + 0);
            }
            return;
        }
    } else {
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
    }
    auto versions = reader.deserialize<IncrementalVersionsRead>("incremental versions");
    if (is_newer(socket_versions_.remote_version, versions.remote_new_version)) {
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Detected outdated datagram. Stored: " << (socket_versions_.remote_version + 0) <<
                ", received: " << versions.remote_base_version;
        }
        return;
    }
    socket_versions_.local.remote_version = versions.local_remote_version;
    socket_versions_.remote_version = versions.remote_new_version;
    if (any(verbosity_ & IoVerbosity::METADATA)) {
        linfo() << "receive versions " << versions;
    }

    std::unordered_set<LocalObjectId> objects_known_and_owned_by_home;
    {
        auto ndeleted = reader.read_binary<NDeletedType>("#deleted");
        for (NDeletedType i = 0; i < ndeleted; ++i) {
            auto id = reader.deserialize<RemoteObjectId>("deleted ID");
            objects_->try_remove(id);
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
    auto receive_local = [&](RemoteObjectVisibility visibility){
        const auto& deleted_objects = objects_->deleted_objects();
        // linfo() << "Received " << object_count << " objects_";
        auto transmission_history_reader = TransmissionHistoryReader{*home_scene_level, objects_->local_time()};
        while (true) {
            auto transmitted_fields = reader.read_binary<TransmittedFields>("transmitted fields");
            if (transmitted_fields == TransmittedFields::NONE) {
                break;
            }
            auto i = transmission_history_reader.read_remote_object_id(reader, transmitted_fields);
            if (i.site_id == home_site_id_) {
                objects_known_and_owned_by_home.insert(i.object_id);
            }
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
                auto lifetime_status = deleted_objects.contains_key(i)
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
                    objects_unknown_here_.erase(i);
                }
            }
        }
    };
    receive_local(RemoteObjectVisibility::PRIVATE);
    receive_local(RemoteObjectVisibility::PUBLIC);
    receive_local(RemoteObjectVisibility::PUBLIC);
    {
        std::vector<LocalObjectId> objects_to_be_deleted;
        objects_to_be_deleted.reserve(objects_known_and_owned_by_home.size());
        for (auto& [i, o] : objects_->public_remote_objects()) {
            if ((i.site_id == home_site_id_) &&
                !objects_known_and_owned_by_home.contains(i.object_id))
            {
                objects_to_be_deleted.push_back(i.object_id);
            }
        }
        for (auto i : objects_to_be_deleted) {
            objects_->try_remove({home_site_id_, i});
        }
    }
}

void IncrementalCommunicatorProxy::send_home(std::iostream& iostr) {
    std::optional<RemoteObjectId> object_to_send_completely;
    {
        std::optional<int32_t> highest_priority;
        auto update_object_to_send_completely_local = [&](const LocalObjects& objects){
            for (const auto& [i, o] : objects) {
                auto j = RemoteObjectId{objects_->local_site_id(), i};
                if (!objects_unknown_at_home_.contains(j)) {
                    continue;
                }
                if (!object_to_send_completely.has_value() || (o->priority() > *highest_priority)) {
                    object_to_send_completely.emplace(j);
                    highest_priority.emplace(o->priority());
                }
            }
        };
        auto update_object_to_send_completely_remote = [&](const RemoteObjects& objects){
            for (const auto& [i, o] : objects) {
                if (!objects_unknown_at_home_.contains(i)) {
                    continue;
                }
                if (!object_to_send_completely.has_value() || (o->priority() > *highest_priority)) {
                    object_to_send_completely.emplace(i);
                    highest_priority.emplace(o->priority());
                }
            }
        };
        update_object_to_send_completely_local(objects_->private_local_objects());
        update_object_to_send_completely_local(objects_->public_local_objects());
        update_object_to_send_completely_remote(objects_->public_remote_objects());
    }
    auto writer = BinaryBitwiseWordsWriter{iostr, nullptr};
    writer.write_binary(session_id_, "session ID");
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
        socket_versions_.local.local_version = std::max(DatagramIndexType(1), ++socket_versions_.local.local_version);
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
            const auto& deleted = objects_->deleted_objects();
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Delete " << deleted.size() << " objects";
            }
            writer.write_binary(integral_cast<NDeletedType>(deleted.size()), "#ndeleted");
            for (const auto& [id, time] : deleted) {
                writer.serialize(id, "deleted ID");
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
        bool new_object_sent = false;
        auto send_local = [&](const LocalObjects& objects){
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Maybe send " << objects.size() << " local objects";
            }
            auto transmission_history_writer = TransmissionHistoryWriter{objects_->local_time(), datagram_counter_};
            for (auto& [i, o] : objects) {
                auto j = RemoteObjectId{objects_->local_site_id(), i};
                auto known_fields = objects_unknown_at_home_.contains(j)
                    ? KnownFields::NONE
                    : KnownFields::ALL;
                if (known_fields == KnownFields::NONE) {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Maybe send complete object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                    if (new_object_sent || (object_to_send_completely.has_value() && (j != *object_to_send_completely))) {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << "Not sending object";
                        }
                        continue;
                    }
                    new_object_sent = true;
                } else {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Maybe send partial object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                }
                o->write(writer, home_site_id_, j, tasks_, known_fields, proxy_objects_caches_.get(), versions, transmission_history_writer);
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
            auto transmission_history_writer = TransmissionHistoryWriter{objects_->local_time(), datagram_counter_};
            for (auto& [i, o] : objects) {
                auto known_fields = objects_unknown_at_home_.contains(i)
                    ? KnownFields::NONE
                    : KnownFields::ALL;
                if (known_fields == KnownFields::NONE) {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Maybe send complete object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                    if (new_object_sent || (object_to_send_completely.has_value() && (i != *object_to_send_completely))) {
                        if (any(verbosity_ & IoVerbosity::METADATA)) {
                            linfo() << "Not sending object";
                        }
                        continue;
                    }
                    new_object_sent = true;
                } else {
                    if (any(verbosity_ & IoVerbosity::METADATA)) {
                        linfo() << "Maybe send partial object to home site " << (home_site_id_ + 0) << ", " << i << " \"" << o->name() << '"';
                    }
                }
                o->write(writer, home_site_id_, i, tasks_, known_fields, proxy_objects_caches_.get(), versions, transmission_history_writer);
            }
            writer.write_binary(TransmittedFields::NONE, "transmitted fields EOF");
        } else {
            send_zero("remote");
        }
    }
    writer.flush_partial("before send");
    send_socket_->send(iostr);
    ++datagram_counter_;
}
