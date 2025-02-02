#pragma once
#include <Mlib/Memory/Destruction_Functions_Removeal_Tokens_Object.hpp>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace Mlib {

class IPlayer;

class Drivers {
public:
    using PlayersMap = std::map<std::string, DestructionFunctionsTokensObject<IPlayer>>;

    Drivers();
    ~Drivers();
    void set_roles(std::vector<std::string> roles);
    bool role_exists(const std::string& role) const;
    bool role_is_free(const std::string& role);
    void add(std::string role, DanglingBaseClassRef<IPlayer> player, SourceLocation loc);
    DanglingBaseClassPtr<IPlayer> try_get(const std::string& role) const;
    void clear();
    const std::string* first_free_role() const;
    const std::string* next_free_role(const std::string& current_role) const;
    const PlayersMap& players_map() const;
private:
    std::vector<std::string> roles_;
    std::set<std::string> roles_set_;
    PlayersMap players_;
};

}
