#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Io/Binary.hpp>
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

void IncrementalCommunicatorProxy::receive_from_home(std::istream& istr) {
    auto receive_local = [&](RemoteObjectVisibility visibility){
        // linfo() << "Received " << object_count << " objects_";
        auto transmission_history_reader = TransmissionHistoryReader();
        while (true) {
            auto transmitted_fields = read_binary<TransmittedFields>(istr, "transmitted fields", verbosity_);
            if (transmitted_fields == TransmittedFields::NONE) {
                break;
            }
            auto i = transmission_history_reader.read(istr, transmitted_fields, verbosity_);
            if (auto it = objects_->try_get(i); it != nullptr) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " read frome home site " << home_site_id_ << ", object " << i;
                }
                it->read(istr, transmitted_fields);
            } else {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " create from home site " << home_site_id_ << ", object " << i;
                }
                auto o = shared_object_factory_->try_create_shared_object(istr, transmitted_fields, i);
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
    auto send_local = [&](const LocalObjects& objects){
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Maybe send " << objects.size() << " local objects";
        }
        auto transmission_history_writer = TransmissionHistoryWriter();
        for (auto& [i, o] : objects) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Maybe send object to home site " << home_site_id_ << ", " << i;
            }
            auto j = RemoteObjectId{objects_->local_site_id(), i};
            auto& known_fields = known_fields_[j];
            o->write(iostr, j, tasks_, known_fields, transmission_history_writer);
        }
        write_binary(iostr, TransmittedFields::NONE, "transmitted fields EOF");
    };
    auto send_zero = [&](const char* msg){
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Send no " << msg << " objects";
        }
        write_binary(iostr, TransmittedFields::NONE, "transmitted fields EOF");
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
        auto transmission_history_writer = TransmissionHistoryWriter();
        for (auto& [i, o] : objects) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Maybe send object to home site " << home_site_id_ << ", " << i;
            }
            auto& known_fields = known_fields_[i];
            o->write(iostr, i, tasks_, known_fields, transmission_history_writer);
        }
        write_binary(iostr, TransmittedFields::NONE, "transmitted fields EOF");
    } else {
        send_zero("remote");
    }
    send_socket_->send(iostr);
}
