#include "Pod_Bots.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Interfaces/Damageable.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Pod_Bot/bot_globals.h>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/mlib.hpp>
#include <Mlib/Scene_Graph/Delete_Node_Mutex.hpp>

using namespace Mlib;

PodBots::PodBots(
    AdvanceTimes& advance_times,
    Players& players,
    CollisionQuery& collision_query,
    DeleteNodeMutex& delete_node_mutex)
: advance_times_{advance_times},
  start_time_{ std::chrono::steady_clock::now() },
  delete_node_mutex_{delete_node_mutex}
{
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    advance_times_.add_advance_time(this);
    pod_bot_set_players(players, collision_query);
}

PodBots::~PodBots() {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    advance_times_.delete_advance_time(this);
    pod_bot_clear_players();
    BotFreeAllMemory();
    gpGlobals->time = 0.f;
    gpGlobals->frametime = 0.f;
}

void PodBots::advance_time(float dt) {
    delete_node_mutex_.assert_this_thread_is_deleter_thread();
    std::chrono::duration<float> elapsed_seconds = std::chrono::steady_clock::now() - start_time_;
    gpGlobals->time = elapsed_seconds.count();
    gpGlobals->frametime = elapsed_seconds.count();
    if (gpGlobals->maxClients > 32) {
        throw std::runtime_error("maxClients too large");
    }
    int player_indices[32];
    g_iNum_bots = 0;
    g_iAliveTs = 0;
    g_iAliveCTs = 0;
    // Determine active bots
    for (int player_index = 0; player_index < gpGlobals->maxClients; ++player_index)
    {
        bot_t& bot = bots[player_index];
        client_t& client = clients[player_index];

        // Overwritten below
        bot.bDead = true;
        client.iFlags &= ~CLIENT_ALIVE;

        if (!FNullEnt(bot.pEdict)) {
            // Overwritten below
            bot.pEdict->v.deadflag = DEAD_DEAD;
            bot.pEdict->v.health = 0;

            if (bot.is_used)
            {
                Player& player = pod_bot_edict_to_player(bot.pEdict);
                if (player.has_rigid_body()) {
                    player_indices[g_iNum_bots++] = player_index;
                }
                bot.pEdict->v.flags |= FL_CLIENT;
                client.iFlags |= CLIENT_USED;

//              if ((pPlayer->v.flags & FL_FAKECLIENT) && (clients[player_index].iFlags & CLIENT_ALIVE)) // KWo - 23.03.2012 - thanks to Immortal_BLG
//                  pPlayer->v.light_level = Light::R_LightPoint (pPlayer->v.origin);

                if ((client.iTeam == TEAM_CS_TERRORIST) && (client.iFlags & CLIENT_ALIVE)) // KWo - 19.01.2008
                    g_iAliveTs++;
                if ((client.iTeam == TEAM_CS_COUNTER) && (client.iFlags & CLIENT_ALIVE))    // KWo - 19.01.2008
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
        }
    }
    // Copy information from Mlib -> PodBot
    for (int bot_rel_index = 0; bot_rel_index < g_iNum_bots; bot_rel_index++)
    {
        bot_t& bot = bots[player_indices[bot_rel_index]];
        client_t& client = clients[player_indices[bot_rel_index]];
        Player& player = pod_bot_edict_to_player(bot.pEdict);
        const RigidBodyPulses& rbp = player.rigid_body().rbi_.rbp_;
        bot.pEdict->v.origin = p_o2q(rbp.abs_position());
        bot.pEdict->v.angles = UTIL_VecToAngles(p_o2q(-rbp.abs_z()));
        bot.pEdict->v.v_angle = UTIL_VecToAngles(p_o2q(player.gun_direction()));
        bot.pEdict->v.absmin = bot.pEdict->v.origin + ::Vector{ -0.5f, -0.5f, -1.f } * s_o2q;
        bot.pEdict->v.size = ::Vector{ 1.f, 1.f, 1.8f } * s_o2q;
        bot.pEdict->v.velocity = p_o2q(rbp.v_);
        bot.pEdict->v.view_ofs = VIEW_OFS;
        bot.pEdict->v.punchangle = player.punch_angle() * 180.f / float{ M_PI };
        bot.pEdict->v.maxspeed = 100.f;
        bot.pEdict->v.fov = 130.f;
        bot.pEdict->v.light_level = 100;
        bot.pEdict->v.health = player.rigid_body().damageable_->health();
        if (sum(squared(rbp.v_)) > 1.f) {
            bot.pEdict->v.movetype = MOVETYPE_WALK;
        } else {
            bot.pEdict->v.movetype = MOVETYPE_NONE;
        }
        if (bot.pEdict->v.health > 0) {
            bot.bDead = false;
            bot.pEdict->v.deadflag = DEAD_NO;
            client.iFlags |= CLIENT_ALIVE;
        }
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
        bot_t& bot = bots[player_indices[bot_rel_index]];
        bot.not_started = false;
        if (pod_bot_edict_to_player(bot.pEdict).game_mode() != GameMode::POD_BOT_PC) {
            g_i_botthink_index = player_indices[bot_rel_index]; // KWo - 02.05.2006
            BotThink (&bot);
        }
    }
}
