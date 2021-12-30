#include "Set_Pod_Bot_Game_Mode.hpp"
#include <Mlib/Players/Pod_Bot/bot_globals.h>

using namespace Mlib;

void Mlib::set_pod_bot_game_mode(const std::string& mode) {
    if (mode == "as") {
        g_iMapType = MAP_AS;
    } else if (mode == "cs") {
        g_iMapType = MAP_CS;
    } else if (mode == "de") {
        g_iMapType = MAP_DE;
    } else if (mode == "awp") {
        g_iMapType = MAP_AWP;
    } else if (mode == "aim") {
        g_iMapType = MAP_AIM;
    } else if (mode == "fy") {
        g_iMapType = MAP_FY;
    } else if (mode == "es") {
        g_iMapType = MAP_ES;
    } else {
        throw std::runtime_error("Unknown PodBot game mode: \"" + mode + '"');
    }
}
