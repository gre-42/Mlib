#pragma once
#include <Mlib/Memory/Event_Emitter.hpp>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace Mlib {

class MacroLineExecutor;
class AssetReferences;
class RemoteSites;
class UserInfo;

class LateJoinPlayerFactory {
public:
    explicit LateJoinPlayerFactory(
        const std::string& filename,
        const MacroLineExecutor& macro_line_executor,
        const AssetReferences& asset_references,
        RemoteSites& remote_sites);
private:
    std::unordered_map<uint32_t, std::function<void()>> create_rank_player_;
    EventReceiverDeletionToken<const UserInfo&> on_user_loaded_level_token_;
};

}
