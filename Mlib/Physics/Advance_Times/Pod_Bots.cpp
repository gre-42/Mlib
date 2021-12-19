#include "Pod_Bots.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <pod_bot/bot_globals.h>
#include <valve/mlib_compat.h>

using namespace Mlib;

PodBots::PodBots(AdvanceTimes& advance_times, Players& players)
: advance_times_{advance_times},
  start_time_{ std::chrono::steady_clock::now() }
{
    advance_times_.add_advance_time(*this);
    pod_bot_set_players(&players);
}

PodBots::~PodBots() {
    advance_times_.schedule_delete_advance_time(this);
}

void PodBots::advance_time(float dt) {
    std::chrono::duration<float> elapsed_seconds = std::chrono::steady_clock::now() - start_time_;
    gpGlobals->time = elapsed_seconds.count();
    gpGlobals->frametime = elapsed_seconds.count();
    // Go through all active Bots, calling their Think function
    g_iNum_bots = 0;
    for (int bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
    {
       if (bots[bot_index].is_used
           && !FNullEnt (bots[bot_index].pEdict))
       {
            bots[bot_index].not_started = false;
            g_i_botthink_index = bot_index; // KWo - 02.05.2006
            BotThink (&bots[bot_index]);
            g_iNum_bots++;
       }
    }
}
