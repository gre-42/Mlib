#include "Pod_Bot.hpp"
#include <pod_bot/bot_globals.h>
#include <valve/mlib_compat.h>

using namespace Mlib;

PodBot::PodBot(
    const std::string& name,
    const std::string& team)
{
    int bot_skill = 101;
    int bot_personality = 5;
    int bot_team = pod_bot_team_id(team);
    int bot_class = 5;
    const char* bot_name = name.c_str();

    BotCreate(
        bot_skill,
        bot_personality,
        bot_team,
        bot_class,
        bot_name);
    
    edict_t* edict = get_edict(name);

    pod_bot_initialize_edict(edict);

    client_t* client = pod_bot_get_client(edict);
    client->iTeam = bot_team;
    client->iFlags |= CLIENT_ALIVE;
}
