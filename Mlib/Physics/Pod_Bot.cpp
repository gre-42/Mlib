#include "Pod_Bot.hpp"
#include <pod_bot/bot_globals.h>

using namespace Mlib;

PodBot::PodBot(const std::string& name)
: name_{name}
{
    int bot_skill = 101;
    int bot_personality = 5;
    int bot_team = 5;
    int bot_class = 5;
    const char* bot_name = name_.c_str();

    BotCreate(
        bot_skill,
        bot_personality,
        bot_team,
        bot_class,
        bot_name);
}
