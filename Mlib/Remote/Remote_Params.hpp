#pragma once
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <cstddef>
#include <nlohmann/json_fwd.hpp>
#include <string>

namespace Mlib {

struct RemoteParams {
    RemoteSiteId site_id;
    RemoteRole role;
    std::string ip;
    uint16_t port;
};

void from_json(const nlohmann::json& j, RemoteParams& params);
void to_json(nlohmann::json& j, const RemoteParams& params);

}
