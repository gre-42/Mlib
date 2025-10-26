#include "Communicator_Proxies.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>

using namespace Mlib;

CommunicatorProxies::CommunicatorProxies(
    DanglingBaseClassRef<ICommunicatorProxyFactory> communicator_proxy_factory,
    RemoteCommunicatorId location_id)
    : communicator_proxy_factory_{ std::move(communicator_proxy_factory) }
    , location_id_{ location_id }
{}

CommunicatorProxies::~CommunicatorProxies() = default;

void CommunicatorProxies::add_receive_socket(DanglingBaseClassRef<IReceiveSocket> socket) {
    receive_sockets_.emplace(std::move(socket.ptr()), CURRENT_SOURCE_LOCATION);
}

void CommunicatorProxies::add_handshake_socket(DanglingBaseClassRef<ISendSocket> socket)
{
    auto proxy = communicator_proxy_factory_->create_communicator_proxy(socket);
    handshake_communicator_proxies_.emplace_back(std::move(proxy), proxy.loc());
}

void CommunicatorProxies::send_and_receive(TransmissionType transmission_type) {
    send(transmission_type);
    receive();
}

void CommunicatorProxies::send(TransmissionType transmission_type) {
    switch (transmission_type) {
    case TransmissionType::HANDSHAKE:
        for (auto& proxy : handshake_communicator_proxies_) {
            std::stringstream sstr;
            write_binary(sstr, location_id_, "location ID");
            proxy->send_home(sstr);
        }
        return;
    case TransmissionType::UNICAST:
        for (auto& [_, proxy] : unicast_communicator_proxies_) {
            std::stringstream sstr;
            write_binary(sstr, location_id_, "location ID");
            proxy->send_home(sstr);
        }
        return;
    case TransmissionType::MULTICAST:
        for (auto& [_, proxy] : multicast_communicator_proxies_) {
            std::stringstream sstr;
            write_binary(sstr, location_id_, "location ID");
            proxy->send_home(sstr);
        }
        return;
    }
    THROW_OR_ABORT("Unknown transmission type");
}

void CommunicatorProxies::receive() {
    for (auto& s : receive_sockets_) {
        while (true) {
            std::stringstream sstr;
            auto responder = s->try_receive(sstr);
            if (responder == nullptr) {
                break;
            }
            auto communicator_proxy = [&](){
                auto node_id = read_binary<RemoteCommunicatorId>(sstr, "node ID", IoVerbosity::SILENT);
                auto it = unicast_communicator_proxies_.find(node_id);
                if (it == unicast_communicator_proxies_.end()) {
                    auto f = communicator_proxy_factory_->create_communicator_proxy(*responder);
                    auto res = unicast_communicator_proxies_.emplace(node_id, std::move(f), CURRENT_SOURCE_LOCATION);
                    if (!res.second) {
                        verbose_abort("Could not add shared object with ID \"" + std::to_string(node_id) + '"');
                    }
                    return res.first->second.object();
                } else {
                    return it->second.object();
                }
            }();
            // linfo() << "Receive at location " << location_id_;
            communicator_proxy->receive_from_home(sstr);
        }
    }
}

void CommunicatorProxies::print(std::ostream& ostr) const {
    ostr <<
        "#receive_sockets: " << receive_sockets_.size() <<
        " #handshake: " << handshake_communicator_proxies_.size() <<
        " #unicast: " << unicast_communicator_proxies_.size() <<
        " #multicast: " << multicast_communicator_proxies_.size();
}

std::ostream& Mlib::operator << (std::ostream& ostr, const CommunicatorProxies& distributed_system) {
    distributed_system.print(ostr);
    return ostr;
}
