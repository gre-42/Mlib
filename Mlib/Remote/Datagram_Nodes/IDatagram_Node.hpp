#pragma once
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <Mlib/Remote/ISend_Socket.hpp>
#include <cstdint>

namespace Mlib {

class IDatagramNode: public ISendSocket, public IReceiveSocket {
public:
    virtual ~IDatagramNode() = default;
    virtual void start_receive_thread(uint32_t max_stored_received_messages) = 0;
    virtual void bind() = 0;
};

}
