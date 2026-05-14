#include "Remote_Socket.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <ostream>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(hostname);
DECLARE_ARGUMENT(port);
}

void Mlib::from_json(const nlohmann::json& j, RemoteSocket& socket) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    socket.hostname = jv.at<std::string>(KnownArgs::hostname);
    socket.port = jv.at<uint16_t>(KnownArgs::port);
}

void Mlib::to_json(nlohmann::json& j, const RemoteSocket& socket) {
    j[KnownArgs::hostname] = socket.hostname;
    j[KnownArgs::port] = socket.port;
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RemoteSocket& socket) {
    ostr << socket.hostname << ':' << socket.port;
    return ostr;
}
