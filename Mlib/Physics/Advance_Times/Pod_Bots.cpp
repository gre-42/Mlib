#include "Pod_Bots.hpp"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <pod_bot/bot_globals.h>
#include <pod_bot_mlib_compat/compat.h>

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
    // Determine active bots
    for (int client_index = 0; client_index < gpGlobals->maxClients; client_index++)
    {
        bot_t& bot = bots[client_index];
        if (bot.is_used && !FNullEnt(bot.pEdict))
        {
            Player& player = pod_bot_edict_to_player(bot.pEdict);
            if (player.has_rigid_body()) {
                client_indices[g_iNum_bots++] = client_index;
            }
        }
    }
    // Copy information from Mlib -> PodBot
    for (int bot_index = 0; bot_index < g_iNum_bots; bot_index++)
    {
        bot_t& bot = bots[client_indices[bot_index]];
        Player& player = pod_bot_edict_to_player(bot.pEdict);
        const RigidBodyPulses& rbp = player.rigid_body().rbi_.rbp_;
        bot.pEdict->v.origin = p_o2q(rbp.abs_position());
        bot.pEdict->v.angles = UTIL_VecToAngles(p_o2q(-rbp.abs_z()));
        bot.pEdict->v.v_angle = UTIL_VecToAngles(p_o2q(-rbp.abs_z()));
        bot.pEdict->v.absmin = bot.pEdict->v.origin + ::Vector{ -0.5f, -0.5f, -1.f } * s_o2q;
        bot.pEdict->v.size = ::Vector{ 1.f, 1.f, 1.8f } * s_o2q;
        bot.pEdict->v.velocity = p_o2q(rbp.v_);
        bot.pEdict->v.view_ofs = ::Vector{ 0.f, 0.f, 50.f };
        bot.pEdict->v.maxspeed = 100.f;
        bot.pEdict->v.fov = 130.f;
        bot.pEdict->v.light_level = 100;
    }
    // Go through all active Bots, calling their Think function
    for (int bot_index = 0; bot_index < g_iNum_bots; bot_index++)
    {
        bot_t& bot = bots[client_indices[bot_index]];
        bot.not_started = false;
        if (pod_bot_edict_to_player(bot.pEdict).game_mode() != GameMode::POD_BOT_PC) {
            g_i_botthink_index = bot_index; // KWo - 02.05.2006
            BotThink (&bot);
        }
    }
}
