#pragma once
#include <Mlib/Remote/Remote_Role.hpp>
#include <Mlib/Remote/Remote_Site_Id.hpp>
#include <Mlib/Remote/Remote_Socket.hpp>
#include <cstddef>
#include <string>
#include <vector>

namespace Mlib {

struct RemoteParams {
    RemoteSiteId site_id;
    RemoteRole role;
    RemoteSocket socket;
    #ifdef __EMSCRIPTEN__
    std::vector<std::byte> cert_hash;
    #endif
};

}
