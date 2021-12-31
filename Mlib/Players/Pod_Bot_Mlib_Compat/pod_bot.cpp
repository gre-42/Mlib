#include "pod_bot.hpp"
#include <Mlib/Physics/Containers/Collision_Query.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Players/Containers/Players.hpp>
#include <Mlib/Players/Pod_Bot/bot_globals.h>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/mlib.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/primitive_constants.hpp>
#include <filesystem>

#define VEC_VIEW Vector( 0, 0, 28 )

std::map<int, edict_t*> indexent_;
static std::map<edict_t*, int> entindex_;
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
        throw std::runtime_error("No index for entity exists");
    }
    return it->second;
}

int DECAL_INDEX(const char *pszDecalName) {
    auto it = decal_map.insert({pszDecalName, decal_map.size()});
    return it.second;
}

long RANDOM_LONG(long min, long max) {
    return std::rand() % (max - min + 1) + min;
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

static struct DefaultEdictInitializer {
    DefaultEdictInitializer() {
        g_solid_edict = new edict_t();
        strcpy(g_solid_edict->v.classname, "default_solid_xyz");
    }
} default_edict_initializer;

void TRACE_LINE(const Vector& vecSource, const Vector& vecDest, int ignored, const edict_t* pentIgnore, TraceResult* tr) {
    if (g_collision_query == nullptr) {
        throw std::runtime_error("TRACE_LINE without collision query");
    }
    if (Mlib::all(vecSource == vecDest)) {
        tr->fAllSolid = 0.f;
        tr->flFraction = 1.f;
        tr->fStartSolid = 0.f;
        tr->pHit = nullptr;
        // tr->pHit = &null_edict;
        return;
    }
    Vector intersection_point;
    Vector intersection_normal;
    const Mlib::RigidBodyIntegrator* excluded0 = (pentIgnore == nullptr)
        ? nullptr
        : &Mlib::pod_bot_edict_to_player(pentIgnore).rigid_body().rbi_;
    const Mlib::RigidBodyIntegrator* excluded1 = nullptr;
    const Mlib::RigidBodyIntegrator* seen_object = nullptr;
    if (g_collision_query->can_see(
        Mlib::p_q2o(vecSource),
        Mlib::p_q2o(vecDest),
        excluded0,
        excluded1,
        bool(ignored & IGNORE_MONSTERS::ignore_monsters),
        &intersection_point,
        &intersection_normal,
        &seen_object))
    {
        tr->fAllSolid = 0.f;
        tr->flFraction = 1.f;
        tr->fStartSolid = 0.f;
        tr->pHit = nullptr;
    } else {
        tr->fAllSolid = 0.f;
        tr->flFraction = 0.f;
        if (seen_object == nullptr) {
            tr->fStartSolid = 1.f;
            tr->pHit = g_solid_edict;
        } else {
            tr->fStartSolid = 0.f;
            tr->pHit = Mlib::get_edict(Mlib::get_player_name(*seen_object));
        }
        tr->vecEndPos = Mlib::p_o2q(*intersection_point);
        tr->vecPlaneNormal = Mlib::u_o2q(*intersection_normal);
    }
}

void TRACE_HULL(const Vector& vecMidPoint, const Vector& vecTemp, IGNORE_MONSTERS ignore_monsters, HULL hull, edict_t* pEdict, TraceResult* tr) {
    TRACE_LINE(vecMidPoint, vecTemp, ignore_monsters, pEdict, tr);
}

int POINT_CONTENTS(const Vector& vec) {
    return CONTENTS_EMPTY;
}

void EMIT_SOUND_DYN2(edict_t*, int, char const*, float, float, int, float) {
    std::cerr << "EMIT_SOUND_DYN2\n";
}

float CVAR_GET_FLOAT(const std::string& name) {
    if (name == "fps_max") {
        return 60.f;
    } else if (name == "sv_gravity") {
        return 800.f;
    } else if (name == "mp_friendlyfire") {
        return 1.f;
    } else if (name == "mp_footsteps") {
        return 1.f;
    } else if (name == "mp_c4timer") {
        return 40.f;
    } else if (name == "mp_freezetime") {
        return 15.f;
    } else if (name == "sv_skycolor_r") {
        return 0.f;
    } else if (name == "sv_skycolor_g") {
        return 0.f;
    } else if (name == "sv_skycolor_b") {
        return 0.f;
    } else if (name == "mp_flashlight") {
        return 0.f;
    } else if (name == "sv_parachute") {
        return 0.f;
    } else {
        throw std::runtime_error("Unknown float value: \"" + name + '"');
    }
}

const char* CVAR_GET_STRING(const char* key) {
    throw std::runtime_error("CVAR_GET_STRING not implemented");
}

int GET_USER_MSG_ID (plid_t plid, const char* name, int* size) {
    throw std::runtime_error("Not yet implemented");
}

plugin_info_t Plugin_info;

static struct GpGlobalsInitializer {
    GpGlobalsInitializer() {
        gpGlobals = new globalvars_t{
            .maxClients = 32,
            .time = 0,
            .frametime = 0,
            .v_right = Vector{1.f, 0.f, 0.f},
            .v_forward = Vector{0.f, 1.f, 0.f},
            .mapname = "undefined_map_name"
        };
    }
} gp_globals_initializer;

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
    // From: https://developer.valvesoftware.com/wiki/Entity_index
    // "Worldspawn is always entity 0, while indices 1 to <maxplayers> are reserved for players.""
    for (int i = 1; i <= 32; ++i) {
        if (indexent_.insert({ i, fakeclient }).second) {
            auto eit = entindex_.insert({ fakeclient, i });
            if (!eit.second) {
                throw std::runtime_error("Could not insert fake client into entindex");
            }
            return fakeclient;
        }
    }
    throw std::runtime_error("Could not find free client slot for \"" + std::string(fakeclient->v.netname) + '"');
}

void enginefuncs_t::pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, uint8_t impulse, uint8_t msec) {
    if (g_players == nullptr) {
        throw std::runtime_error("pfnRunPlayerMove without previous call to set_players");
    }
    auto& player = g_players->get_player(fakeclient->v.netname);
    if (player.has_rigid_body()) {
        // Vector forward;
        // Vector right;
        // Vector up;
        // AngleVectors(*(const Vector*)viewangles, forward, right, up);
        // Mlib::FixedArray<float, 3, 3> R{
        //     right(0), forward(0), up(0),
        //     right(1), forward(1), up(1),
        //     right(2), forward(2), up(2)};
        // // This can cause nonzero roll-angles, compensated by the other angles.
        // Vector ma = Mlib::matrix_2_tait_bryan_angles(Mlib::mm_q2o(R));
        // player.run_move(
        //     ma(1),
        //     ma(0),
        //     forwardmove,
        //     sidemove);
        player.run_move(
            (viewangles[YAW] - 90.f) * float{M_PI} / 180.f,
            -viewangles[PITCH] * float{M_PI} / 180.f,
            forwardmove,
            sidemove);
        
        if (buttons & IN_ATTACK) {
            player.trigger_gun();
        }

        // if (buttons & IN_ATTACK) std::cerr << "attack" << std::endl;
        // if (buttons & IN_JUMP) std::cerr << "jump" << std::endl;
        // if (buttons & IN_DUCK) std::cerr << "duck" << std::endl;
        // // if (buttons & IN_FORWARD) std::cerr << "forward" << std::endl;
        // if (buttons & IN_BACK) std::cerr << "back" << std::endl;
        // if (buttons & IN_USE) std::cerr << "use" << std::endl;
        // if (buttons & IN_CANCEL) std::cerr << "cancel" << std::endl;
        // if (buttons & IN_LEFT) std::cerr << "left" << std::endl;
        // if (buttons & IN_RIGHT) std::cerr << "right" << std::endl;
        // if (buttons & IN_MOVELEFT) std::cerr << "moveleft" << std::endl;
        // if (buttons & IN_MOVERIGHT) std::cerr << "moveright" << std::endl;
        // if (buttons & IN_ATTACK2) std::cerr << "attack2" << std::endl;
        // if (buttons & IN_RUN) std::cerr << "run" << std::endl;
        // if (buttons & IN_RELOAD) std::cerr << "reload" << std::endl;
        // if (buttons & IN_ALT1) std::cerr << "alt1" << std::endl;
        // if (buttons & IN_SCORE) std::cerr << "score" << std::endl;

    }
}
