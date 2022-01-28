#pragma once

namespace Mlib {

class GameLogic;
class Players;
class Spawn;

class TeamDeathmatch {
    friend GameLogic;
public:
    TeamDeathmatch(
        Players& players,
        Spawn& spawn);
    ~TeamDeathmatch();
private:
    void handle_team_deathmatch();
    Players& players_;
    Spawn& spawn_;
};

}
