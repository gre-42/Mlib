#include "Remote_Role.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>

using namespace std::string_view_literals;
using namespace Mlib;

RemoteRole Mlib::remote_role_from_string(const std::string_view& s) {
    static const std::map<std::string_view, RemoteRole> m{
        {"server"sv, RemoteRole::SERVER},
        {"client"sv, RemoteRole::CLIENT}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown remote role: \"" + std::string(s) + '"');
    }
    return it->second;
}

std::string_view Mlib::remote_role_to_string(RemoteRole r) {
    switch (r) {
    case RemoteRole::SERVER:
        return "server"sv;
    case RemoteRole::CLIENT:
        return "client"sv;
    }
    THROW_OR_ABORT("Unknown remote role: \"" + std::to_string((int)r) + '"');
}
