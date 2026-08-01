#include "Fragmenting_Datagram_Node.hpp"
#include <Mlib/Remote/Network_Transmission_Status.hpp>
#include <Mlib/Remote/Sockets/Fragmenting_Receiver.hpp>
#include <Mlib/Remote/Sockets/Fragmenting_Send_Socket.hpp>
#include <Mlib/Remote/Sockets/Fragmenting_Sender.hpp>

using namespace Mlib;

FragmentingDatagramNode::FragmentingDatagramNode(std::shared_ptr<IDatagramNode> node)
    : node_{std::move(node)}
    , fragmenting_receiver_{std::make_shared<FragmentingReceiveSocket>()}
    , fragmenting_sender_{std::make_shared<FragmentingSender>()}
{
    if (dynamic_cast<FragmentingDatagramNode*>(node_.get()) != nullptr) {
        throw std::runtime_error("Detected recursive datagram fragmentation (0)");
    }
}

void FragmentingDatagramNode::start_receive_thread(uint32_t max_stored_received_messages) {
    node_->start_receive_thread(max_stored_received_messages);
}

void FragmentingDatagramNode::bind() {
    node_->bind();
}

void FragmentingDatagramNode::send(std::istream& istr) {
    fragmenting_sender_->send(istr, *node_);
}

std::shared_ptr<ISendSocket> FragmentingDatagramNode::try_receive(
    std::ostream& ostr,
    NetworkTransmissionStatus& transmission_status)
{
    std::stringstream sstr;
    auto responder = node_->try_receive(sstr, transmission_status);
    if (responder == nullptr) {
        return nullptr;
    }
    if (!fragmenting_receiver_->try_receive(ostr, sstr)) {
        return nullptr;
    }
    return std::make_shared<FragmentingSendSocket>(
        std::move(responder),
        fragmenting_sender_);
}
