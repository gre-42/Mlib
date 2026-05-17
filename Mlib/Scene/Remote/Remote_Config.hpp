#pragma once
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Remote/IReceive_Socket.hpp>
#include <optional>

namespace Mlib {

class RemoteSites;
struct RemoteParams;

struct RemoteConfig {
    const std::optional<RemoteParams>& game;
    #ifdef WITHOUT_GRAPHICS
    DanglingBaseClassRef<IReceiveSocket> admin_socket;
    #endif
};

struct RemoteConfigAndSites {
    RemoteSites& sites;
    RemoteConfig& config;
};

}
