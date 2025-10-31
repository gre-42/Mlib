#include "Communicator_Proxies.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>

using namespace Mlib;

CommunicatorProxies::CommunicatorProxies(
    const DanglingBaseClassRef<ICommunicatorProxyFactory>& communicator_proxy_factory,
    RemoteSiteId site_id)
    : communicator_proxy_factory_{ communicator_proxy_factory }
    , site_id_{ site_id }
{}

CommunicatorProxies::~CommunicatorProxies() = default;

void CommunicatorProxies::add_receive_socket(const DanglingBaseClassRef<IReceiveSocket>& socket) {
    receive_sockets_.emplace(socket.ptr(), CURRENT_SOURCE_LOCATION);
}

void CommunicatorProxies::add_handshake_socket(std::shared_ptr<ISendSocket> socket)
{
    auto proxy = communicator_proxy_factory_->create_communicator_proxy(std::move(socket));
    handshake_communicator_proxies_.emplace_back(proxy, proxy.loc());
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
            write_binary(sstr, site_id_, "location ID");
            proxy->send_home(sstr);
        }
        return;
    case TransmissionType::UNICAST:
        for (auto& [_, proxy] : unicast_communicator_proxies_) {
            std::stringstream sstr;
            write_binary(sstr, site_id_, "location ID");
            proxy->send_home(sstr);
        }
        return;
    case TransmissionType::MULTICAST:
        for (auto& [_, proxy] : multicast_communicator_proxies_) {
            std::stringstream sstr;
            write_binary(sstr, site_id_, "location ID");
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
                auto site_id = read_binary<RemoteSiteId>(sstr, "node ID", IoVerbosity::SILENT);
                auto it = unicast_communicator_proxies_.find(site_id);
                if (it == unicast_communicator_proxies_.end()) {
                    auto f = communicator_proxy_factory_->create_communicator_proxy(std::move(responder));
                    auto res = unicast_communicator_proxies_.emplace(site_id, std::move(f), CURRENT_SOURCE_LOCATION);
                    if (!res.second) {
                        verbose_abort("Could not add communicator proxy with ID \"" + std::to_string(site_id) + '"');
                    }
                    return res.first->second.object();
                } else {
                    it->second->set_send_socket(std::move(responder));
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
