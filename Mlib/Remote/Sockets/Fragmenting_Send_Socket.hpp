#pragma once
#include <Mlib/Remote/ISend_Socket.hpp>
#include <memory>

namespace Mlib {

class ISendSocket;
class FragmentingSender;

class FragmentingSendSocket: public ISendSocket {
public:
    FragmentingSendSocket(
        std::shared_ptr<ISendSocket> socket,
        std::shared_ptr<FragmentingSender> fragmenting_sender);
    virtual void send(std::istream& istr, SendStatusCode& status_code) override;
private:
    std::shared_ptr<ISendSocket> socket_;
    std::shared_ptr<FragmentingSender> fragmenting_sender_;
};

}
