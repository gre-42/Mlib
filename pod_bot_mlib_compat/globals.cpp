#include "globals.hpp"

Mlib::Players* g_players;
Mlib::CollisionQuery* g_collision_query;
std::map<std::string, edict_t*> g_player_name_to_edict;
std::map<edict_t*, std::string> g_edict_to_player_name;
