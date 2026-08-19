#pragma once
#include <Mlib/Memory/Event_Emitter.hpp>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace Mlib {

struct RemoteConfigAndSites;
class MacroLineExecutor;
class AssetReferences;
class Players;
class UserInfo;

class LateJoinPlayerFactory {
    LateJoinPlayerFactory(const LateJoinPlayerFactory&) = delete;
    LateJoinPlayerFactory& operator = (const LateJoinPlayerFactory&) = delete;
public:
    explicit LateJoinPlayerFactory(
        const std::string& filename,
        const MacroLineExecutor& macro_line_executor,
        const AssetReferences& asset_references,
        RemoteConfigAndSites& remote,
        Players& players);
private:
    std::unordered_map<uint32_t, std::function<void()>> create_rank_player_;
    EventReceiverDeletionToken<UserInfo&> on_user_loaded_level_token_;
};

}
