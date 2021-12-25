#pragma once
#include <map>
#include <string>

struct edict_t;
struct globalvars_t;
struct plugin_info_t;

namespace Mlib {

class Player;
class Players;
class CollisionQuery;
class RigidBodyIntegrator;

}

extern Mlib::CollisionQuery* g_collision_query;
extern Mlib::Players* g_players;
extern std::map<edict_t*, std::string> g_edict_to_player_name;
extern std::map<std::string, edict_t*> g_player_name_to_edict;

extern globalvars_t *gpGlobals;
extern plugin_info_t Plugin_info;
#define PLID &Plugin_info
