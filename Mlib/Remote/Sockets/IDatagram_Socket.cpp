#include "IDatagram_Socket.hpp"
#include <Mlib/Os/Env.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void IDatagramSocket::send(std::istream& istr, SendStatusCode& status_code) {
    auto data = read_all_vector(istr, "send buffer", IoVerbosity::SILENT);
    send(data, status_code);
}
