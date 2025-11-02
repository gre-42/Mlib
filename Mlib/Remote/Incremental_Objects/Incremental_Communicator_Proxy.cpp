#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Object_Compression.hpp>

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
        uint32_t local_object_count = read_binary<uint32_t>(istr, "local object count", verbosity_);
        // linfo() << "Received " << object_count << " objects_";
        for (uint32_t j = 0; j < local_object_count; ++j) {
            auto i = read_binary<LocalObjectId>(istr, "instance and type ID", verbosity_);
            if (auto it = objects_->try_get({home_site_id_, i}); it != nullptr) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " read frome home site " << home_site_id_ << ", object " << i;
                }
                it->read(istr);
            } else {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " create from home site " << home_site_id_ << ", object " << i;
                }
                auto o = shared_object_factory_->try_create_shared_object(istr, {home_site_id_, i});
                if (o != nullptr) {
                    objects_->add_remote_object({home_site_id_, i}, *o, visibility);
                }
            }
        }
    };
    receive_local(RemoteObjectVisibility::PRIVATE);
    receive_local(RemoteObjectVisibility::PUBLIC);
    {
        uint32_t remote_object_count = read_binary<uint32_t>(istr, "remote object count", verbosity_);
        // linfo() << "Received " << object_count << " objects_";
        for (uint32_t j = 0; j < remote_object_count; ++j) {
            auto i = read_binary<RemoteObjectId>(istr, "instance and type ID", verbosity_);
            if (auto it = objects_->try_get(i); it != nullptr) {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " read frome remote site " << i;
                }
                it->read(istr);
            } else {
                if (any(verbosity_ & IoVerbosity::METADATA)) {
                    linfo() << this << " create from remote site " << i;
                }
                auto o = shared_object_factory_->try_create_shared_object(istr, i);
                if (o != nullptr) {
                    objects_->add_remote_object(i, *o, RemoteObjectVisibility::PUBLIC);
                }
            }
        }
    }
}

void IncrementalCommunicatorProxy::send_home(std::iostream& iostr) {
    auto send_local = [&](const LocalObjects& objects){
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Send " << objects.size() << " local objects";
        }
        write_binary(iostr, integral_cast<uint32_t>(objects.size()), "local object count");
        for (auto& [i, o] : objects) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Send object to home site " << home_site_id_ << ", " << i;
            }
            write_binary(iostr, i, "instance and type ID");
            if (known_objects.contains({objects_->site_id(), i})) {
                o->write(iostr, ObjectCompression::INCREMENTAL);
            } else {
                o->write(iostr, ObjectCompression::NONE);
            }
        }
    };
    auto send_zero_local = [&](){
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Send no local objects";
        }
        write_binary<uint32_t>(iostr, 0, "local object count (zero)");
    };
    if (any(tasks_ & ProxyTasks::SEND_LOCAL)) {
        send_local(objects_->private_local_objects());
        send_local(objects_->public_local_objects());
    } else {
        send_zero_local();
        send_zero_local();
    }
    if (any(tasks_ & ProxyTasks::SEND_REMOTE)) {
        const auto& objects = objects_->public_remote_objects();
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Send " << objects.size() << " remote objects";
        }
        write_binary(iostr, integral_cast<uint32_t>(objects.size()), "remote object count");
        for (auto& [i, o] : objects) {
            if (any(verbosity_ & IoVerbosity::METADATA)) {
                linfo() << "Send object to home site " << home_site_id_ << ", " << i;
            }
            write_binary(iostr, i, "instance and type ID");
            if (known_objects.contains(i)) {
                o->write(iostr, ObjectCompression::INCREMENTAL);
            } else {
                o->write(iostr, ObjectCompression::NONE);
            }
        }
    } else {
        if (any(verbosity_ & IoVerbosity::METADATA)) {
            linfo() << "Send no remote objects";
        }
        write_binary<uint32_t>(iostr, 0, "remote object count");
    }
    send_socket_->send(iostr);
}
