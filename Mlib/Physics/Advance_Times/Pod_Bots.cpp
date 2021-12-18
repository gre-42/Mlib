#include "Pod_Bots.hpp"
#include <Mlib/Physics/Containers/Advance_Times.hpp>
#include <pod_bot/bot_globals.h>

using namespace Mlib;

PodBots::PodBots(AdvanceTimes& advance_times)
: advance_times_{advance_times}
{
    advance_times_.add_advance_time(*this);
}

PodBots::~PodBots() {
    advance_times_.schedule_delete_advance_time(this);
}

void PodBots::advance_time(float dt) {
    // Go through all active Bots, calling their Think function
    g_iNum_bots = 0;
    for (int bot_index = 0; bot_index < gpGlobals->maxClients; bot_index++)
    {
       if (bots[bot_index].is_used
           && !FNullEnt (bots[bot_index].pEdict))
       {
          g_i_botthink_index = bot_index; // KWo - 02.05.2006
          BotThink (&bots[bot_index]);
          g_iNum_bots++;
       }
    }
}
