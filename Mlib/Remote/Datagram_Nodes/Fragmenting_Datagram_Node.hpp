#pragma once
#include <Mlib/Remote/Datagram_Nodes/IDatagram_Node.hpp>

namespace Mlib {

class FragmentingReceiveSocket;
class FragmentingSender;

class FragmentingDatagramNode: public IDatagramNode {
public:
    explicit FragmentingDatagramNode(std::shared_ptr<IDatagramNode> node);
    virtual void start_receive_thread(uint32_t max_stored_received_messages) override;
    virtual void bind() override;
    virtual void send(std::istream& istr) override;
    virtual std::shared_ptr<ISendSocket> try_receive(
        std::ostream& ostr,
        NetworkTransmissionStatus& transmission_status) override;
private:
    std::shared_ptr<IDatagramNode> node_;
    std::shared_ptr<FragmentingReceiveSocket> fragmenting_receiver_;
    std::shared_ptr<FragmentingSender> fragmenting_sender_;
};

}
