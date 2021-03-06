#pragma once
#include <Mlib/Players/Pod_Bot_Mlib_Compat/globals.hpp>
#include <Mlib/Players/Pod_Bot_Mlib_Compat/types.hpp>
#include <cstdint>
#include <map>
#include <string>

// extern ::Vector g_dest_origin[32];

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

enum IGNORE_MONSTERS {
    dont_ignore_monsters = 0,
    ignore_monsters = 1,
    ignore_glass = 2
};

void TRACE_LINE (const ::Vector& vecSource, const ::Vector& vecDest, int ignored, const edict_t* dct, TraceResult* tr);

enum HULL {
    head_hull,
    human_hull,
    point_hull
};

void TRACE_HULL (const ::Vector& vecMidPoint, const ::Vector& vecTemp, IGNORE_MONSTERS ignore_monsters, HULL hull, edict_t* pEdict, TraceResult* tr);

int POINT_CONTENTS(const ::Vector& vec);

::Vector UTIL_VecToAngles(const ::Vector& v);
::Vector GetGunPosition(const edict_t* dct);
void MAKE_VECTORS(const ::Vector& v);

inline ::Vector2D Make2D(const ::Vector& v) {
    return ::Vector2D(v(0), v(1));
}

template <size_t tndim>
float DotProduct(const Mlib::FixedArray<float, tndim>& a, const Mlib::FixedArray<float, tndim>& b) {
    return Mlib::dot0d(a, b);
}

template <size_t tndim>
float Length(const Mlib::FixedArray<float, tndim>& v) {
    return std::sqrt(Mlib::dot0d(v, v));
}

template <size_t tndim>
Mlib::FixedArray<float, tndim> Normalize(const Mlib::FixedArray<float, tndim>& v) {
    return v / Length(v);
}

edict_t* FIND_ENTITY_IN_SPHERE (edict_t* pent, const ::Vector& origin, float radius);

inline float Length2D(const ::Vector& v) {
    return Length(Make2D(v));
}

void EMIT_SOUND_DYN2 (edict_t* pent, int  channel, const char*, float volume, float attn, int undefined, float pitch);

inline bool IS_DEDICATED_SERVER() {
    return false;
}

inline void MESSAGE_BEGIN (int priority, MESSAGE msg, void* unknown = nullptr, edict_t* pHostEdict = nullptr) {}
inline void WRITE_CHAR(char) {}
inline void WRITE_BYTE(uint8_t) {}
inline void WRITE_SHORT(uint8_t) {}
inline void WRITE_COORD(float v) {}
inline void MESSAGE_END() {}

static const int SF_PUSH_BREAKABLE = 0;
static const int SF_BREAK_TRIGGER_ONLY = 1;

float CVAR_GET_FLOAT(const std::string& name);
int GET_USER_MSG_ID (plid_t plid, const char* name, int* size);

inline edict_t* FIND_ENTITY_BY_CLASSNAME(edict_t* pStartEntity, const char* name) {
    throw std::runtime_error("FIND_ENTITY_BY_CLASSNAME not implemented");
    return nullptr;
}

const char* CVAR_GET_STRING(const char* key);

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

int COMPARE_FILE_TIME(const std::string& filename1, const std::string& filename2, int* iCompare);

void SERVER_COMMAND(const char* cmd);

inline void SET_ORIGIN (edict_t* edict, const ::Vector& v) {
    edict->v.origin = v;
}
