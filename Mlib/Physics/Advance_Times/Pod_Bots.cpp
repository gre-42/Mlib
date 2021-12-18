#include "Pod_Bots.hpp"
#include <podbot/bot_globals.h>

using namespace Mlib;

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
