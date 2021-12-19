#include "mlib_compat.h"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <filesystem>
#include <stdarg.h>

edict_t* INDEXENT(int index) {
    static std::map<int, edict_t*> map;
    return map[index];
}

int ENTINDEX(const edict_t* e) {
    static std::map<const edict_t*, int> map;
    auto it = map.insert({ e, map.size() });
    return it.second;
}

int DECAL_INDEX(const char *pszDecalName) {
    static std::map<std::string, int> map;
    auto it = map.insert({pszDecalName, map.size()});
    return it.second;
}

long RANDOM_LONG(long min, long max) {
    return std::rand() % (max - min) + min;
}

float RANDOM_FLOAT(float min, float max) {
    return (float)rand()/(float)(RAND_MAX/(max - min)) + min;
}

int COMPARE_FILE_TIME(const std::string& filename1, const std::string& filename2, int* iCompare) {
    std::error_code ec;
    std::filesystem::file_time_type t1 = last_write_time(std::filesystem::path{ filename1 }, ec);
    if (ec.value() != 0) {
        return 0;
    }
    std::filesystem::file_time_type t2 = last_write_time(std::filesystem::path{ filename2 }, ec);
    if (ec.value() != 0) {
        return 0;
    }
    if (t1 < t2) {
        *iCompare = -1;
    }
    if (t1 > t2) {
        *iCompare = 1;
    }
    *iCompare = 0;
    return 1;
}

void SERVER_COMMAND(const char* cmd) {
    std::cerr << "SERVER_COMMAND: " << cmd << std::endl;
}

edict_t* FIND_ENTITY_IN_SPHERE (edict_t* pent, const Vector& origin, float radius) {
    throw std::runtime_error("Not yet implemented");
}

void UTIL_ServerPrint(const char *fmt, ...) {
    /* Declare a va_list type variable */
    va_list myargs;

    /* Initialise the va_list variable with the ... after fmt */

    va_start(myargs, fmt);

    /* Forward the '...' to vprintf */
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "UTIL_ServerPrint failed" << std::endl;
    }

    /* Clean up the va_list */
    va_end(myargs);
}

void FakeClientCommand(edict_t*, char const*, ...) {
    throw std::runtime_error("Not yet implemented");
}

void MAKE_VECTORS(Mlib::FixedArray<float, 3ul> const&) {
    throw std::runtime_error("Not yet implemented");
}

void TRACE_LINE(Mlib::FixedArray<float, 3ul> const&, Mlib::FixedArray<float, 3ul> const&, int, edict_t const*, TraceResult*) {
    throw std::runtime_error("Not yet implemented");
}

void UTIL_HostPrint(char const*, ...) {
    throw std::runtime_error("Not yet implemented");
}

void EMIT_SOUND_DYN2(edict_t*, int, char const*, float, float, int, float) {
    throw std::runtime_error("Not yet implemented");
}

void TRACE_HULL(Mlib::FixedArray<float, 3ul> const&, Mlib::FixedArray<float, 3ul> const&, IGNORE_MONSTERS, HULL, edict_t*, TraceResult*) {
    throw std::runtime_error("Not yet implemented");
}

void UTIL_HudMessage(edict_t*, hudtextparms_t const&, char*) {
    throw std::runtime_error("Not yet implemented");
}

int POINT_CONTENTS(const Vector& vec) {
    throw std::runtime_error("Not yet implemented");
}

int GET_USER_MSG_ID (plid_t plid, const char* name, int* size) {
    throw std::runtime_error("Not yet implemented");
}

plugin_info_t Plugin_info;

globalvars_t* gpGlobals = new globalvars_t{
    .maxClients = 32,
    .time = 0,
    .frametime = 0,
    .v_right = Vector{1.f, 0.f, 0.f},
    .v_forward = Vector{0.f, 1.f, 0.f},
    .mapname = "undefined_map_name"
};

bool FNullEnt(const edict_t* pent) {
    return pent == nullptr;
}

static Mlib::Players* g_players;
static std::map<edict_t*, Mlib::Player*> g_player_map;

edict_t* enginefuncs_t::pfnCreateFakeClient(const char* name) {
    edict_t* result = new edict_t;
    strncpy(result->v.netname, name, sizeof(result->v.netname));
    result->v.netname[sizeof(result->v.netname) - 1] = '\0';
    result->v.health = 100;
    result->v.flags = 0;
    return result;
}

void enginefuncs_t::pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, uint8_t impulse, uint8_t msec) {
    if (g_players == nullptr) {
        throw std::runtime_error("pfnRunPlayerMove without previous call to set_players");
    }
    auto& player = g_players->get_player(fakeclient->v.netname);
    if (player.has_rigid_body()) {
        player.run_move(forwardmove, sidemove);
    }
}

void Mlib::pod_bot_set_players(Players* players) {
    if (g_players != nullptr) {
        throw std::runtime_error("Players already set");
    }
    g_players = players;
}

int Mlib::pod_bot_team_id(const std::string& team_name) {
    std::map<std::string, int> team_ids_;
    auto it = team_ids_.insert({ team_name, team_ids_.size() });
    return it.second;
}
