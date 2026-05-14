#pragma once
#include <cstdint>
#include <iosfwd>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

struct RemoteSocket {
    std::string hostname;
    uint16_t port;
};

void from_json(const nlohmann::json& j, RemoteSocket& socket);
void to_json(nlohmann::json& j, const RemoteSocket& socket);

std::ostream& operator << (std::ostream& ostr, const RemoteSocket& socket);

}
