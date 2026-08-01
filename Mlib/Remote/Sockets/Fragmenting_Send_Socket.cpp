#include "Fragmenting_Send_Socket.hpp"
#include <Mlib/Remote/Sockets/Fragmenting_Sender.hpp>

using namespace Mlib;

FragmentingSendSocket::FragmentingSendSocket(
    std::shared_ptr<ISendSocket> socket,
    std::shared_ptr<FragmentingSender> fragmenting_sender)
    : socket_{std::move(socket)}
    , fragmenting_sender_{std::move(fragmenting_sender)}
{
    if (dynamic_cast<FragmentingSendSocket*>(socket_.get()) != nullptr) {
        throw std::runtime_error("Detected recursive datagram fragmentation (1)");
    }
}

void FragmentingSendSocket::send(std::istream& istr) {
    fragmenting_sender_->send(istr, *socket_);
}
