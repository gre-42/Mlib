#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Math.hpp>
// #include <metamod/metamod/plinfo.h>
#include <cstdlib>
#include <map>
#include <string>

typedef Mlib::FixedArray<float, 2> Vector2D;
typedef Mlib::FixedArray<float, 3> Vector;
typedef Mlib::FixedArray<float, 3> vec3_t;

struct edict_t;

struct entvars_t{
    int flags;
    int effects;
    int weapons;
    int solid;
    int light_level;
    Vector origin;
    Vector angles;
    Vector v_angle;
    Vector absmin;
    Vector velocity;
    Vector view_ofs;
    bool takedamage;
    int impulse;
    int dmg;
    int movetype;
    int waterlevel;
    int weaponanim;
    int deadflag;
    int playerclass;
    int spawnflags;
    float armorvalue;
    float health;
    float maxspeed;
    float ideal_yaw;
    float idealpitch;
    float yaw_speed;
    float pitch_speed;
    float fov;
    Vector punchangle;
    uint32_t frags;
    int button;
    int oldbuttons;
    char netname[32];
    char classname[32];
    const char* model = nullptr;
    char viewmodel[32];
    const char* target = nullptr;
    const char* targetname = nullptr;
    edict_t* owner = nullptr;
    edict_t* dmg_inflictor = nullptr;
    edict_t* groundentity = nullptr;
    double dmgtime;
    int renderfx;
    int rendermode;
    Vector rendercolor;
    Vector size;
    float renderamt;
};

struct edict_t {
    entvars_t v;
    std::map<std::string, const char*> info_key_buffer;
    void* pvPrivateData = nullptr;
};

inline char* GET_INFOKEYBUFFER(edict_t* edict) {
    return reinterpret_cast<char*>(&edict->info_key_buffer);
}

inline const char* INFOKEY_VALUE (const char* info_key_buffer, const std::string& key) {
    auto* b = reinterpret_cast<const std::map<std::string, const char*>*>(info_key_buffer);
    auto it = b->find(key);
    return (it == b->end())
        ? nullptr
        : it->second;
}

static const int MAX_AMMO_SLOTS = 32; // total ammo amounts (1 array for each bot)
static const int MAX_WEAPONS = 32;

typedef struct hudtextparms_s
{
    float        x;
    float        y;
    int          effect;
    uint8_t      r1, g1, b1, a1;
    uint8_t      r2, g2, b2, a2;
    float        fadeinTime;
    float        fadeoutTime;
    float        holdTime;
    float        fxTime;
    int          channel;
} hudtextparms_t;

static const bool TRUE = true;
static const bool FALSE = false;
static const Vector g_vecZero{0.f, 0.f, 0.f};

long RANDOM_LONG(long min, long max);
float RANDOM_FLOAT(float min, float max);

edict_t* INDEXENT(int index);

#define eoNullEntity 0
// inline BOOL FNullEnt(EOFFSET eoffset)            { return eoffset == 0; }
bool FNullEnt(const edict_t* pent);
// inline BOOL FNullEnt(entvars_t* pev)                { return pev == NULL || FNullEnt(OFFSET(pev)); }

typedef const char *string_t;
#define NULL_STRING                0
#define STRING( c_str )            ( c_str )
#define MAKE_STRING( c_str )       ( c_str )
#define IDENT_STRINGS( s1, s2 )    *((void **)&(s1)) == *((void **)&(s2))

inline bool FStrEq(const char *sz1, const char *sz2)
{
    return(strcmp(sz1, sz2) == 0);
}

#define FREE_PRIVATE(entity) free(entity->pvPrivateData)

#define CALL_GAME_ENTITY(PLID, class_name, v)

int ENTINDEX(edict_t* e);

int DECAL_INDEX(const char *pszDecalName);

inline void SET_CLIENT_KEYVALUE (int index, char* infobuffer, const std::string& key, const char* value) {
    auto* b = reinterpret_cast<std::map<std::string, const char*>*>(infobuffer);
    (*b)[key] = value;
}

inline void MDLL_ClientConnect(edict_t* BotEnt, const char* c_name, const char* ip, char ptr[128])
{}

inline void MDLL_ClientPutInServer(edict_t* BotEnt)
{}

typedef enum { ignore_monsters=1, dont_ignore_monsters=0, missile=2 } IGNORE_MONSTERS;
typedef enum { ignore_glass=1 } IGNORE_GLASS;

Vector UTIL_VecToAngles(const Vector& v);

struct TraceResult{
    bool fStartSolid;
    Vector vecEndPos;
    float flFraction;
    float fAllSolid;
    Vector vecPlaneNormal;
    edict_t* pHit;
};

void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceLine (const Vector &vecStart, const Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);

Vector GetGunPosition(const edict_t* dct);
void TRACE_LINE (const Vector& vecSource, const Vector& vecDest, int ignored, const edict_t* dct, TraceResult* tr);

void MAKE_VECTORS(const Vector& v);

inline Vector2D Make2D(const Vector& v) {
    return Vector2D(v(0), v(1));
}

template <size_t tndim>
float DotProduct(const Mlib::FixedArray<float, tndim>& a, const Mlib::FixedArray<float, tndim>& b) {
    return Mlib::sum(a * b);
}

template <size_t tndim>
float Length(const Mlib::FixedArray<float, tndim>& v) {
    return std::sqrt(Mlib::sum(Mlib::squared(v)));
}

template <size_t tndim>
Mlib::FixedArray<float, tndim> Normalize(const Mlib::FixedArray<float, tndim>& v) {
    return v / Length(v);
}

edict_t* FIND_ENTITY_IN_SPHERE (edict_t* pent, const Vector& origin, float radius);

inline float Length2D(const Vector& v) {
    return Length(Make2D(v));
}

void EMIT_SOUND_DYN2 (edict_t* pent, int  channel, const char*, float volume, float attn, int undefined, float pitch);

struct globalvars_t {
    int maxClients;
    float time;
    float frametime;
    Vector v_right;
    Vector v_forward;
    Vector v_up;
    const char* mapname;
};

extern globalvars_t* gpGlobals;

inline bool IS_DEDICATED_SERVER() {
    return false;
}

struct cvar_t{
    const char* name;
    const char* string;
    int flags;
    float value;
    cvar_t(const char* name, const char* string, int flags)
    : name{name},
      string{string},
      flags{flags},
      value(atof(string))
    {}
};

static const Vector VEC_HULL_MIN{0.f, 0.f, 1.8f};
static const Vector VEC_DUCK_HULL_MIN{0.f, 0.f, 1.f};

int POINT_CONTENTS(const Vector& vec);

enum HULL {head_hull, human_hull, point_hull};

void TRACE_HULL (const Vector& vecMidPoint, const Vector& vecTemp, IGNORE_MONSTERS ignore_monsters, HULL hull, edict_t* pEdict, TraceResult* tr);

enum MESSAGE {
    SVC_TEMPENTITY = 23,
    SVC_INTERMISSION = 30,
    SVC_CDTRACK = 32,
    SVC_WEAPONANIM = 35,
    SVC_ROOMTYPE = 37,
    SVC_HLTV = 50
};

inline void MESSAGE_BEGIN (int priority, MESSAGE msg, void* unknown = nullptr, edict_t* pHostEdict = nullptr) {}
inline void WRITE_CHAR(char) {}
inline void WRITE_BYTE(uint8_t) {}
inline void WRITE_SHORT(uint8_t) {}
inline void WRITE_COORD(float v) {}
inline void MESSAGE_END() {}

static const int SF_PUSH_BREAKABLE = 0;
static const int SF_BREAK_TRIGGER_ONLY = 1;

inline float CVAR_GET_FLOAT(const std::string& name) {
    throw std::runtime_error("Unknown float value: \"" + name + '"');
}

// Information plugin provides about itself.
struct plugin_info_t{};

// Plugin identifier, passed to all Meta Utility Functions.
typedef plugin_info_t* plid_t;

extern plugin_info_t Plugin_info;

#define PLID    &Plugin_info

int GET_USER_MSG_ID (plid_t plid, const char* name, int* size);

inline edict_t* FIND_ENTITY_BY_CLASSNAME(edict_t* pStartEntity, const char* name) {
    throw std::runtime_error("FIND_ENTITY_BY_CLASSNAME not implemented");
    return nullptr;
}

inline const char* CVAR_GET_STRING(const char* key) {
    return key;
}

inline void PLAYER_CNX_STATS(const edict_t *pClient, int *ping, int *packet_loss) {
    *ping = 42;
    *packet_loss = 43;
}

inline edict_t* FIND_ENTITY_BY_STRING(edict_t *pEdictStartSearchAfter, const char *pszField, const char *pszValue) {
    throw std::runtime_error("FIND_ENTITY_BY_STRING not implemented");
}

inline int GETENTITYILLUM(edict_t *pEnt) {
    throw std::runtime_error("GETENTITYILLUM not implemented");
}

struct enginefuncs_t{
    edict_t* pfnCreateFakeClient(const char* name);
    void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, uint8_t impulse, uint8_t msec);
};

typedef enum {
    MRES_UNSET = 0,
    MRES_IGNORED,        // plugin didn't take any action
    MRES_HANDLED,        // plugin did something, but real function should still be called
    MRES_OVERRIDE,       // call real function, but use my return value
    MRES_SUPERCEDE,      // skip real function; use my return value
} META_RES;

int COMPARE_FILE_TIME(const std::string& filename1, const std::string& filename2, int* iCompare);

void SERVER_COMMAND(const char* cmd);

inline void SET_ORIGIN (edict_t* edict, const Vector& v) {
    edict->v.origin = v;
}

static const int FCVAR_ARCHIVE = 1;          // Save this cvar.
static const int FCVAR_USERINFO = 2;         // Changes the client's info string.
static const int FCVAR_SERVER = 4;           // Notifies players when changed.
static const int FCVAR_EXTDLL = 8;           // Defined by external dll.
static const int FCVAR_CLIENTDLL = 16;       // Defined by the client dll.
static const int FCVAR_PROTECTED = 32;       // It's a server cvar, but we don't send the data since it's a password, etc.
static const int FCVAR_SPONLY = 64;          // This cvar cannot be changed by clients connected to a multiplayer server.
static const int FCVAR_PRINTABLEONLY = 128;  // This cvar's string cannot contain unprintable characters (e.g. used for player name etc)
static const int FCVAR_UNLOGGED = 256;       // If this is a FCVAR_SERVER, don't log changes to the log file / console if we are creating a log

namespace Mlib {

class Player;
class Players;
class CollisionQuery;
class RigidBodyIntegrator;

void pod_bot_set_players(Players& players, CollisionQuery& collision_query);

int pod_bot_team_id(const std::string& team_name);

Player& pod_bot_edict_to_player(const edict_t* edict);

void set_player_rigid_body_integrator(const RigidBodyIntegrator& rbi, const std::string& player_name);

std::string get_player_name(const RigidBodyIntegrator& rbi);

edict_t* get_edict(const std::string& player_name);

void pod_bot_initialize_edict(edict_t* edict);

}
