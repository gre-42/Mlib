#include "Pod_Bot.hpp"
#include <pod_bot/bot_globals.h>
#include <pod_bot_mlib_compat/mlib.hpp>

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

    auto cb = pod_bot_get_client_and_bot(edict);

    int weapon = CS_WEAPON_M4A1;
    int ammo = 999;

    cb.client->iTeam = bot_team;
    cb.client->iFlags |= CLIENT_ALIVE;
    cb.client->iCurrentWeaponId = weapon;

    cb.bot->current_weapon.iId = weapon;
    cb.bot->m_rgAmmoInClip[weapon] = ammo;
    cb.bot->current_weapon.iClip = ammo;
    cb.bot->current_weapon.iAmmo1 = 0;
    cb.bot->current_weapon.iAmmo2 = 0;

    edict->v.weapons = (1 << weapon);
}
