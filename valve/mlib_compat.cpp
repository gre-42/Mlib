#include "mlib_compat.h"
#include <Mlib/Physics/Advance_Times/Player.hpp>
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Containers/Players.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <filesystem>
#include <stdarg.h>

#define VEC_VIEW Vector( 0, 0, 28 )

static std::map<int, edict_t*> indexent_;
static std::map<edict_t*, int> entindex_;
static Mlib::Players* g_players;
static Mlib::CollisionQuery* g_collision_query;
static std::map<edict_t*, std::string> g_edict_to_player_name;
static std::map<std::string, edict_t*> g_player_name_to_edict;
static std::map<const Mlib::RigidBodyIntegrator*, std::string> g_rbi_to_player_name;
static std::map<std::string, int> decal_map;

edict_t* INDEXENT(int index) {
    auto it = indexent_.find(index);
    if (it == indexent_.end()) {
        throw std::runtime_error("No entity with index exists");
    }
    return it->second;
}

int ENTINDEX(edict_t* e) {
    auto it = entindex_.find(e);
    if (it == entindex_.end()) {
        throw std::runtime_error("Not index for entity exists");
    }
    return it->second;
}

int DECAL_INDEX(const char *pszDecalName) {
    auto it = decal_map.insert({pszDecalName, decal_map.size()});
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
    std::map<int, edict_t*>::iterator it;
    if (pent == nullptr) {
        it = indexent_.begin();
    } else {
        int start_index = ENTINDEX(pent);
        it = indexent_.find(start_index);
        if (it == indexent_.end()) {
            throw std::runtime_error("Could not find start entity");
        }
        ++it;
    }
    for (; it != indexent_.end(); ++it) {
        if (sum(squared(it->second->v.origin - origin)) < Mlib::squared(radius)) {
            return it->second;
        }
    }
    return nullptr;
}

void UTIL_ServerPrint(const char *fmt, ...) {
    // Declare a va_list type variable
    va_list myargs;

    // Initialise the va_list variable with the ... after fmt
    va_start(myargs, fmt);

    // Forward the '...' to vprintf
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "UTIL_ServerPrint failed" << std::endl;
    }

    // Clean up the va_list
    va_end(myargs);
}

void FakeClientCommand(edict_t* edict, char const* fmt, ...) {
    // Declare a va_list type variable
    va_list myargs;

    // Initialise the va_list variable with the ... after fmt
    va_start(myargs, fmt);

    // Forward the '...' to vprintf
    if (vprintf(fmt, myargs) < 0) {
        std::cerr << "FakeClientCommand failed" << std::endl;
    }

    // Clean up the va_list
    va_end(myargs);
}

// up / down
#define PITCH 0
// left / right
#define YAW   1
// fall over
#define ROLL  2 

void AngleVectors (const Vector& angles, Vector& forward, Vector& right, Vector& up)
{
    float angle;
    float sr, sp, sy, cr, cp, cy;
    
    angle = angles(YAW) * (M_PI*2 / 360);
    sy = sin(angle);
    cy = cos(angle);
    angle = angles(PITCH) * (M_PI*2 / 360);
    sp = sin(angle);
    cp = cos(angle);
    angle = angles(ROLL) * (M_PI*2 / 360);
    sr = sin(angle);
    cr = cos(angle);

    forward(0) = cp*cy;
    forward(1) = cp*sy;
    forward(2) = -sp;

    right(0) = (-1*sr*sp*cy+-1*cr*-sy);
    right(1) = (-1*sr*sp*sy+-1*cr*cy);
    right(2) = -1*sr*cp;

    up(0) = (cr*sp*cy+-sr*-sy);
    up(1) = (cr*sp*sy+-sr*cy);
    up(2) = cr*cp;
}

void pfnMakeVectors(const Vector& angles)
{
	AngleVectors(angles, gpGlobals->v_forward, gpGlobals->v_right, gpGlobals->v_up);
}

void MAKE_VECTORS(const Vector& angles) {
    pfnMakeVectors(angles);
}

Mlib::Player& Mlib::pod_bot_edict_to_player(const edict_t* edict) {
    auto pit = g_edict_to_player_name.find(const_cast<edict_t*>(edict));
    if (pit == g_edict_to_player_name.end()) {
        throw std::runtime_error("Could not find player for entity");
    }
    return g_players->get_player(pit->second);
}

void TRACE_LINE(const Vector& vecSource, const Vector& vecDest, int ignored, const edict_t* pentIgnore, TraceResult* tr) {
    if (g_collision_query == nullptr) {
        throw std::runtime_error("TRACE_LINE without collision query");
    }
    Vector intersection_point;
    Vector intersection_normal;
    Mlib::Player& player = Mlib::pod_bot_edict_to_player(pentIgnore);
    const Mlib::RigidBodyIntegrator* excluded0 = &player.rigid_body().rbi_;
    const Mlib::RigidBodyIntegrator* excluded1 = nullptr;
    const Mlib::RigidBodyIntegrator* seen_object = nullptr;
    if (g_collision_query->can_see(
        vecSource,
        vecDest,
        excluded0,
        excluded1,
        bool(ignored & IGNORE_MONSTERS::ignore_monsters),
        &intersection_point,
        &intersection_normal,
        &seen_object))
    {
        tr->fAllSolid = 0.f;
        tr->flFraction = 0.f;
        tr->fStartSolid = 0.f;
        tr->pHit = nullptr;
    } else {
        tr->fAllSolid = 1.f;
        tr->flFraction = 1.f;
        tr->fStartSolid = 1.f;
        if (seen_object == nullptr) {
            tr->pHit = nullptr;
        } else {
            tr->pHit = Mlib::get_edict(Mlib::get_player_name(*seen_object));
        }
        tr->vecEndPos = *intersection_point;
        tr->vecPlaneNormal = *intersection_normal;
    }
}

void UTIL_HostPrint(char const*, ...) {
    throw std::runtime_error("Not yet implemented");
}

void EMIT_SOUND_DYN2(edict_t*, int, char const*, float, float, int, float) {
    throw std::runtime_error("Not yet implemented");
}

void TRACE_HULL(const Vector& vecMidPoint, const Vector& vecTemp, IGNORE_MONSTERS ignore_monsters, HULL hull, edict_t* pEdict, TraceResult* tr) {
    TRACE_LINE(vecMidPoint, vecTemp, ignore_monsters, pEdict, tr);
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

edict_t* enginefuncs_t::pfnCreateFakeClient(const char* name) {
    if (g_players == nullptr) {
        throw std::runtime_error("pfnCreateFakeClient without previous call to set_players");
    }
    edict_t* fakeclient = new edict_t;
    strncpy(fakeclient->v.netname, name, sizeof(fakeclient->v.netname));
    fakeclient->v.netname[sizeof(fakeclient->v.netname) - 1] = '\0';
    fakeclient->v.health = 100;
    fakeclient->v.flags = 0;
    fakeclient->v.view_ofs = VEC_VIEW;
    strcpy(fakeclient->v.classname, "player");
    strcpy(fakeclient->v.viewmodel, "undefined_viewmodel");
    if (!g_edict_to_player_name.insert({ fakeclient, fakeclient->v.netname }).second) {
        throw std::runtime_error("Could not insert into edict to player map");
    }
    if (!g_player_name_to_edict.insert({ fakeclient->v.netname, fakeclient }).second) {
        throw std::runtime_error("Could not insert into player to edict map");
    }
    auto eit = entindex_.insert({ fakeclient, entindex_.size() });
    if (!eit.second) {
        throw std::runtime_error("Could not insert fake client into entindex");
    }
    if (!indexent_.insert({ eit.first->second, fakeclient }).second) {
        throw std::runtime_error("Could not insert fake client into indexent");
    }
    return fakeclient;
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

void Mlib::pod_bot_set_players(Players& players, CollisionQuery& collision_query) {
    if (g_players != nullptr) {
        throw std::runtime_error("Players already set");
    }
    if (g_collision_query != nullptr) {
        throw std::runtime_error("Collision query already set");
    }
    g_players = &players;
    g_collision_query = &collision_query;
}

int Mlib::pod_bot_team_id(const std::string& team_name) {
    std::map<std::string, int> team_ids_;
    auto it = team_ids_.insert({ team_name, team_ids_.size() });
    return it.second;
}

void Mlib::set_player_rigid_body_integrator(
    const Mlib::RigidBodyIntegrator& rbi,
    const std::string& player_name)
{
    if (!g_rbi_to_player_name.insert({ &rbi, player_name }).second) {
        throw std::runtime_error("Could not set player rigid body integrator");
    }
}

std::string Mlib::get_player_name(const Mlib::RigidBodyIntegrator& rbi) {
    auto it = g_rbi_to_player_name.find(&rbi);
    if (it == g_rbi_to_player_name.end()) {
        throw std::runtime_error("Could not find player name for rigid body integrator");
    }
    return it->second;
}

edict_t* Mlib::get_edict(const std::string& player_name) {
    auto it = g_player_name_to_edict.find(player_name);
    if (it == g_player_name_to_edict.end()) {
        throw std::runtime_error("Could not find edict for player name");
    }
    return it->second;
}

void Mlib::pod_bot_initialize_edict(edict_t* edict) {
    edict->v.absmin = edict->v.origin - ::Vector{ 0.5f, 0.5f, 1.f };
    edict->v.size = ::Vector{ 1.f, 1.f, 1.8f };
    const size_t PRIVATE_DATA_SIZE = 1000 * 1000;
    edict->pvPrivateData = malloc(PRIVATE_DATA_SIZE);
    std::fill((char*)edict->pvPrivateData, (char*)edict->pvPrivateData + PRIVATE_DATA_SIZE, 0);
}
