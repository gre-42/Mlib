#include "Communicator_Proxy.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <Mlib/Remote/Object_Compression.hpp>

using namespace Mlib;

CommunicatorProxy::CommunicatorProxy(
    DanglingBaseClassRef<ISendSocket> socket,
    DanglingBaseClassRef<ISharedObjectFactory> shared_object_factory)
    : socket_{ std::move(socket) }
    , shared_object_factory_{ std::move(shared_object_factory) }
{}

CommunicatorProxy::~CommunicatorProxy() {
    on_destroy.clear();
}

void CommunicatorProxy::receive_from_home(SharedObjects& objects, std::istream& istr) {
    uint32_t object_count = read_binary<uint32_t>(istr, "object count", IoVerbosity::SILENT);
    // linfo() << "Received " << object_count << " objects";
    for (uint32_t j = 0; j < object_count; ++j) {
        auto i = read_binary<RemoteObjectId>(istr, "instance and type ID", IoVerbosity::SILENT);
        if (auto it = objects.find(i); it != objects.end()) {
            it->second->read(istr);
        } else {
            auto o = shared_object_factory_->create_shared_object(istr);
            if (!objects.emplace(i, std::move(o), CURRENT_SOURCE_LOCATION).second) {
                verbose_abort("Could not add shared object with ID \"" + std::to_string(i) + '"');
            }
        }
    }
}

void CommunicatorProxy::send_home(const SharedObjects& objects, std::iostream& iostr) {
    // linfo() << "Send " << objects.size() << " objects";
    write_binary(iostr, integral_cast<uint32_t>(objects.size()), "object count");
    for (auto& [i, o] : objects) {
        write_binary(iostr, i, "instance and type ID");
        if (known_objects.contains(i)) {
            o->write(iostr, ObjectCompression::INCREMENTAL);
        } else {
            o->write(iostr, ObjectCompression::NONE);
        }
    }
    socket_->send(iostr);
}
