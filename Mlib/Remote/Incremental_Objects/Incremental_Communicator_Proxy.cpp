#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Io/Binary_Reader.hpp>
#include <Mlib/Io/Binary_Writer.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Incremental_Objects/Proxy_Tasks.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmission_History.hpp>
#include <Mlib/Remote/Incremental_Objects/Transmitted_Fields.hpp>

using namespace Mlib;

IncrementalCommunicatorProxy::IncrementalCommunicatorProxy(
    std::shared_ptr<ISendSocket> send_socket,
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects,
    IoVerbosity verbosity,
    ProxyTasks tasks,
    RemoteSiteId home_site_id)
    : send_socket_{ std::move(send_socket) }
    , shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
    , verbosity_{ verbosity }
    , tasks_{ tasks }
    , home_site_id_{ home_site_id }
{}

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
    THROW_OR_ABORT("Unknown scene level load status");
}

void IncrementalCommunicatorProxy::receive_from_home(std::istream& istr) {
    auto reader = BinaryReader{istr, verbosity_};
    {
        auto scene_level_name = reader.read_string("scene level name");
        auto reload_count = reader.read_binary<uint8_t>("reload_count");
        home_scene_level_.emplace(std::move(scene_level_name), reload_count);
        auto level_selector = objects_->local_scene_level_selector();
        if (any(tasks_ & ProxyTasks::RELOAD_SCENE)) {
            if (level_selector->reload_required(*home_scene_level_)) {
                level_selector->client_schedule_load_scene_level(*home_scene_level_);
                return;
            }
        }
        auto home_load_level_status = reader.read_binary<LocalSceneLevelLoadStatus>("scene level load status");
        if (is_loading(level_selector->load_status()) ||
            is_loading(home_load_level_status))
        {
            return;
        }
    }
    {
        auto ndeleted = reader.read_binary<uint16_t>("#deleted");
        for (uint16_t i = 0; i < ndeleted; ++i) {
            auto id = reader.read_binary<RemoteObjectId>("deleted ID");
            objects_->try_remove(id);
        }
    }
    auto receive_local = [&](RemoteObjectVisibility visibility){
        // linfo() << "Received " << object_count << " objects_";
        auto transmission_history_reader = TransmissionHistoryReader{*home_scene_level_, objects_->local_time()};
        while (true) {
            auto transmitted_fields = read_binary<TransmittedFields>(istr, "transmitted fields", verbosity_);
            if (transmitted_fields == TransmittedFields::NONE) {
                break;
            }
            auto i = transmission_history_reader.read_remote_object_id(istr, transmitted_fields, verbosity_);
            if (auto it = objects_->try_get(i); it != nullptr) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " read frome home site " << home_site_id_ << ", object " << i;
                }
                it->read(istr, i, tasks_, transmitted_fields, transmission_history_reader);
            } else {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " create from home site " << home_site_id_ << ", object " << i;
                }
                auto o = shared_object_factory_->try_create_shared_object(
                    istr, i, tasks_, transmitted_fields, transmission_history_reader);
                if (o != nullptr) {
                    objects_->add_remote_object(i, *o, visibility);
                }
            }
        }
    };
    receive_local(RemoteObjectVisibility::PRIVATE);
    receive_local(RemoteObjectVisibility::PUBLIC);
    receive_local(RemoteObjectVisibility::PUBLIC);
}

void IncrementalCommunicatorProxy::send_home(std::iostream& iostr) {
    auto writer = BinaryWriter{iostr};
    [&](){
        {
            auto level_selector = objects_->local_scene_level_selector();
            auto level = level_selector->get_local_scene_level();
            writer.write_string(level.name, "level name");
            writer.write_binary(level.reload_count, "reload count");
            writer.write_binary(level_selector->load_status(), "scene level load status");
            if (is_loading(level_selector->load_status())) {
                return;
            }
        }
        {
            const auto& deleted = objects_->deleted_objects();
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Delete " << deleted.size() << " objects";
            }
            writer.write_binary(integral_cast<uint16_t>(deleted.size()), "#ndeleted");
            for (const auto& [id, time] : deleted) {
                writer.write_binary(id, "deleted ID");
            }
        }
        auto send_local = [&](const LocalObjects& objects){
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Maybe send " << objects.size() << " local objects";
            }
            auto transmission_history_writer = TransmissionHistoryWriter{objects_->local_time()};
            for (auto& [i, o] : objects) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Maybe send object to home site " << home_site_id_ << ", " << i;
                }
                auto j = RemoteObjectId{objects_->local_site_id(), i};
                auto& known_fields = known_fields_[j];
                o->write(iostr, j, tasks_, known_fields, transmission_history_writer);
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
            auto transmission_history_writer = TransmissionHistoryWriter{objects_->local_time()};
            for (auto& [i, o] : objects) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << "Maybe send object to home site " << home_site_id_ << ", " << i;
                }
                auto& known_fields = known_fields_[i];
                o->write(iostr, i, tasks_, known_fields, transmission_history_writer);
            }
            writer.write_binary(TransmittedFields::NONE, "transmitted fields EOF");
        } else {
            send_zero("remote");
        }
    }();
    send_socket_->send(iostr);
}
