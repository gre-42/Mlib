#pragma once
#include <functional>

namespace Mlib {

class GameLogic;
class Players;
class Spawn;

class TeamDeathmatch {
    friend GameLogic;
public:
    TeamDeathmatch(
        Players& players,
        Spawn& spawn,
        const std::function<void()>& setup_new_round);
    ~TeamDeathmatch();
private:
    void handle_team_deathmatch();
    Players& players_;
    Spawn& spawn_;
    std::function<void()> setup_new_round_;
};

}
