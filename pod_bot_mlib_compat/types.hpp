#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <map>
#include <pod_bot_mlib_compat/primitive_constants.hpp>

typedef Mlib::FixedArray<float, 2> Vector2D;
typedef Mlib::FixedArray<float, 3> Vector;
typedef Mlib::FixedArray<float, 3> vec3_t;

struct edict_t;
struct client_t;

struct entvars_t{
    int flags;
    int effects;
    int weapons;
    int solid;
    int light_level;
    ::Vector origin;
    ::Vector angles;
    ::Vector v_angle;
    ::Vector absmin;
    ::Vector velocity;
    ::Vector view_ofs;
    ::Vector punchangle;
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
    ::Vector rendercolor;
    ::Vector size;
    float renderamt;
};

struct edict_t {
    entvars_t v;
    std::map<std::string, const char*> info_key_buffer;
    void* pvPrivateData = nullptr;
};


struct mplane_t {
    ::Vector normal;
    float dist;
};

struct mtexinfo_t{
    float vecs[2][4];
};

struct color24 {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

struct msurface_t{
    int flags;
    mtexinfo_t *texinfo;
    uint16_t texturemins[2];
    uint16_t extents[2];
    color24 *samples;
    uint8_t styles[MAXLIGHTMAPS];
};

typedef int qboolean;

struct decal_t {};

struct mnode_t{
    int contents;
    mplane_t *plane;
    mnode_t *children[2];
    uint16_t firstsurface;
    uint16_t numsurfaces;
};

struct model_t {
    int	numnodes;
    mnode_t	*nodes;
    color24	*lightdata;
    int numsurfaces;
	msurface_t *surfaces;
};

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

struct enginefuncs_t{
    edict_t* pfnCreateFakeClient(const char* name);
    void pfnRunPlayerMove(edict_t *fakeclient, const float *viewangles, float forwardmove, float sidemove, float upmove, unsigned short buttons, uint8_t impulse, uint8_t msec);
};

struct globalvars_t {
    int maxClients;
    float time;
    float frametime;
    ::Vector v_right;
    ::Vector v_forward;
    ::Vector v_up;
    const char* mapname;
};

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

struct plugin_info_t{};

typedef plugin_info_t* plid_t;

struct TraceResult{
    bool fStartSolid;
    ::Vector vecEndPos;
    float flFraction;
    float fAllSolid;
    ::Vector vecPlaneNormal;
    edict_t* pHit;
};
