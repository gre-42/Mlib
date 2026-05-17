#pragma once
#include <cstddef>
#include <system_error>
#include <vector>

namespace Mlib {

class IDatagramSocket {
public:
    virtual ~IDatagramSocket() = default;
    virtual void open() = 0;
    virtual void bind() = 0;
    virtual void shutdown(std::error_code& ec) = 0;
    virtual void close() = 0;
    virtual size_t receive(
        std::vector<std::byte>& receive_buffer,
        std::shared_ptr<IDatagramSocket>& reply_socket,
        std::error_code& ec) = 0;
    virtual size_t send(
        const std::vector<std::byte>& data,
        std::error_code& ec) = 0;
};

}
