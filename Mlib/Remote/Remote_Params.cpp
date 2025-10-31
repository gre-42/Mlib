#include "Remote_Params.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Remote/Remote_Role.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(site_id);
DECLARE_ARGUMENT(role);
DECLARE_ARGUMENT(ip);
DECLARE_ARGUMENT(port);
}

void Mlib::from_json(const nlohmann::json& j, RemoteParams& params) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    params.site_id = jv.at<RemoteSiteId>(KnownArgs::site_id);
    params.role = remote_role_from_string(jv.at<std::string>(KnownArgs::role));
    params.ip = jv.at<std::string>(KnownArgs::ip);
    params.port = jv.at<uint16_t>(KnownArgs::port);
}

void Mlib::to_json(nlohmann::json& j, const RemoteParams& params) {
    j[KnownArgs::site_id] = params.site_id;
    j[KnownArgs::role] = remote_role_to_string(params.role);
    j[KnownArgs::ip] = params.ip;
    j[KnownArgs::port] = params.port;
}
