#include "Pod_Bots.hpp"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <pod_bot/bot_globals.h>
#include <pod_bot_mlib_compat/mlib.hpp>

using namespace Mlib;

PodBots::PodBots(
    AdvanceTimes& advance_times,
    Players& players,
    CollisionQuery& collision_query)
: advance_times_{advance_times},
  start_time_{ std::chrono::steady_clock::now() }
{
    advance_times_.add_advance_time(*this);
    pod_bot_set_players(players, collision_query);
}

PodBots::~PodBots() {
    advance_times_.schedule_delete_advance_time(this);
}

void PodBots::advance_time(float dt) {
    std::chrono::duration<float> elapsed_seconds = std::chrono::steady_clock::now() - start_time_;
    gpGlobals->time = elapsed_seconds.count();
    gpGlobals->frametime = elapsed_seconds.count();
    if (gpGlobals->maxClients > 32) {
        throw std::runtime_error("maxClients too large");
    }
    int client_indices[32];
    g_iNum_bots = 0;
    g_iAliveTs = 0;
    g_iAliveCTs = 0;
    // Determine active bots
    for (int client_index = 0; client_index < gpGlobals->maxClients; client_index++)
    {
        bot_t& bot = bots[client_index];
        if (!FNullEnt(bot.pEdict)) {
            if (bot.is_used)
            {
                Player& player = pod_bot_edict_to_player(bot.pEdict);
                if (player.has_rigid_body()) {
                    client_indices[g_iNum_bots++] = client_index;
                }
                bot.pEdict->v.flags |= FL_CLIENT | CLIENT_USED;

//              if ((pPlayer->v.flags & FL_FAKECLIENT) && (clients[player_index].iFlags & CLIENT_ALIVE)) // KWo - 23.03.2012 - thanks to Immortal_BLG
//                  pPlayer->v.light_level = Light::R_LightPoint (pPlayer->v.origin);

                if ((clients[client_index].iTeam == TEAM_CS_TERRORIST) && (clients[client_index].iFlags & CLIENT_ALIVE)) // KWo - 19.01.2008
                    g_iAliveTs++;
                if ((clients[client_index].iTeam == TEAM_CS_COUNTER) && (clients[client_index].iFlags & CLIENT_ALIVE))    // KWo - 19.01.2008
                    g_iAliveCTs++;

//              if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used)
//                 && ((!g_i_cv_BotsQuotaMatch) || (clients[player_index].iTeam == TEAM_CS_TERRORIST)
//                      || (clients[player_index].iTeam == TEAM_CS_COUNTER))) // KWo - 16.10.2006
//                 g_iNum_humans++;
//
//              if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used)
//                 && ((clients[player_index].iTeam == TEAM_CS_TERRORIST)
//                      || (clients[player_index].iTeam == TEAM_CS_COUNTER))) // KWo - 08.03.2010
//                 g_iNum_hum_tm++;
//
//               if (!(pPlayer->v.flags & FL_FAKECLIENT) && !(bots[player_index].is_used) && (clients[player_index].iFlags & CLIENT_ALIVE) && g_b_cv_autokill) // KWo - 02.05.2006
//               {
//                  bAliveHumans = true;
//      //            if (g_iFrameCounter == 10)
//      //               UTIL_ServerPrint("[Debug] Player %s is alive.\n", STRING(pPlayer->v.netname));
//               }
            }
            if (bot.is_used && (bot.pEdict->v.health > 0)) {
                bot.bDead = false;
                bot.pEdict->v.deadflag = DEAD_NO;
                clients[client_index].iFlags |= CLIENT_ALIVE;
            } else {
                bot.bDead = true;
                bot.pEdict->v.deadflag = DEAD_DEAD;
                clients[client_index].iFlags &= ~CLIENT_ALIVE;
            }
        }
    }
    // Copy information from Mlib -> PodBot
    for (int bot_rel_index = 0; bot_rel_index < g_iNum_bots; bot_rel_index++)
    {
        bot_t& bot = bots[client_indices[bot_rel_index]];
        Player& player = pod_bot_edict_to_player(bot.pEdict);
        const RigidBodyPulses& rbp = player.rigid_body().rbi_.rbp_;
        bot.pEdict->v.origin = p_o2q(rbp.abs_position());
        bot.pEdict->v.angles = UTIL_VecToAngles(p_o2q(-rbp.abs_z()));
        bot.pEdict->v.v_angle = UTIL_VecToAngles(p_o2q(player.gun_direction()));
        bot.pEdict->v.absmin = bot.pEdict->v.origin + ::Vector{ -0.5f, -0.5f, -1.f } * s_o2q;
        bot.pEdict->v.size = ::Vector{ 1.f, 1.f, 1.8f } * s_o2q;
        bot.pEdict->v.velocity = p_o2q(rbp.v_);
        bot.pEdict->v.view_ofs = VIEW_OFS;
        bot.pEdict->v.punchangle = ::Vector{ 0.f, 0.f, 0.f };
        bot.pEdict->v.maxspeed = 100.f;
        bot.pEdict->v.fov = 130.f;
        bot.pEdict->v.light_level = 100;
    }
    g_iFrameCounter++;
    if (g_iFrameCounter >= gpGlobals->maxClients) {
       g_iFrameCounter = 0;
    }
    if (g_f_cvars_upd_time <= gpGlobals->time) {
       UTIL_CheckCvars();
       g_f_cvars_upd_time = gpGlobals->time + 1.f;
    }
    // Go through all active Bots, calling their Think function
    for (int bot_rel_index = 0; bot_rel_index < g_iNum_bots; bot_rel_index++)
    {
        bot_t& bot = bots[client_indices[bot_rel_index]];
        bot.not_started = false;
        if (pod_bot_edict_to_player(bot.pEdict).game_mode() != GameMode::POD_BOT_PC) {
            g_i_botthink_index = client_indices[bot_rel_index]; // KWo - 02.05.2006
            BotThink (&bot);
        }
    }
}
