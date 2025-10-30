#include "Incremental_Communicator_Proxy.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object.hpp>
#include <Mlib/Remote/Incremental_Objects/IIncremental_Object_Factory.hpp>
#include <Mlib/Remote/Object_Compression.hpp>

using namespace Mlib;

IncrementalCommunicatorProxy::IncrementalCommunicatorProxy(
    const DanglingBaseClassRef<ISendSocket>& socket,
    const DanglingBaseClassRef<IIncrementalObjectFactory>& shared_object_factory,
    const DanglingBaseClassRef<IncrementalRemoteObjects>& objects)
    : socket_{ socket }
    , shared_object_factory_{ shared_object_factory }
    , objects_{ objects }
{}

IncrementalCommunicatorProxy::~IncrementalCommunicatorProxy() {
    on_destroy.clear();
}

void IncrementalCommunicatorProxy::receive_from_home(std::istream& istr) {
    uint32_t object_count = read_binary<uint32_t>(istr, "object count", IoVerbosity::SILENT);
    // linfo() << "Received " << object_count << " objects_";
    for (uint32_t j = 0; j < object_count; ++j) {
        auto i = read_binary<RemoteObjectId>(istr, "instance and type ID", IoVerbosity::SILENT);
        if (auto it = objects_->try_get(i); it != nullptr) {
            it->read(istr);
        } else {
            auto o = shared_object_factory_->create_shared_object(istr);
            objects_->add_remote_object(i, std::move(o));
        }
    }
}

void IncrementalCommunicatorProxy::send_home(std::iostream& iostr) {
    // linfo() << "Send " << objects_.size() << " objects_";
    write_binary(iostr, integral_cast<uint32_t>(objects_->size()), "object count");
    for (auto& [i, o] : objects_.get()) {
        write_binary(iostr, i, "instance and type ID");
        if (known_objects.contains(i)) {
            o->write(iostr, ObjectCompression::INCREMENTAL);
        } else {
            o->write(iostr, ObjectCompression::NONE);
        }
    }
    socket_->send(iostr);
}
