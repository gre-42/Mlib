#include "IDatagram_Socket.hpp"
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void IDatagramSocket::send(std::istream& istr) {
    auto data = read_all_vector(istr, "send buffer", IoVerbosity::SILENT);
    std::error_code ec;
    auto sent = send(data, ec);
    if (getenv_default_bool("NET_DEBUG", false)) {
        linfo() << this << " send_to. Error: " << (int)(bool)ec << ", Length: " << sent << " / " << data.size();
    }
    if (ec) {
        throw std::runtime_error("Send failed: \"" + ec.message() + '"');
    }
    if (sent != data.size()) {
        throw std::runtime_error((std::stringstream() << "Bytes sent: " << sent << ". Expected: " << data.size()).str());
    }
}
