#pragma once
#include <cstdint>
#include <map>
#include <pod_bot_mlib_compat/globals.hpp>
#include <pod_bot_mlib_compat/types.hpp>
#include <string>

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

typedef enum { ignore_monsters=1, dont_ignore_monsters=0, missile=2 } IGNORE_MONSTERS;
typedef enum { ignore_glass=1 } IGNORE_GLASS;

::Vector UTIL_VecToAngles(const ::Vector& v);

void UTIL_TraceLine (const ::Vector &vecStart, const ::Vector &vecEnd, IGNORE_MONSTERS igmon, IGNORE_GLASS ignoreGlass, edict_t *pentIgnore, TraceResult *ptr);
void UTIL_TraceLine (const ::Vector &vecStart, const ::Vector &vecEnd, IGNORE_MONSTERS igmon, edict_t *pentIgnore, TraceResult *ptr);

::Vector GetGunPosition(const edict_t* dct);
void TRACE_LINE (const ::Vector& vecSource, const ::Vector& vecDest, int ignored, const edict_t* dct, TraceResult* tr);

void MAKE_VECTORS(const ::Vector& v);

inline ::Vector2D Make2D(const ::Vector& v) {
    return ::Vector2D(v(0), v(1));
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

edict_t* FIND_ENTITY_IN_SPHERE (edict_t* pent, const ::Vector& origin, float radius);

inline float Length2D(const ::Vector& v) {
    return Length(Make2D(v));
}

void EMIT_SOUND_DYN2 (edict_t* pent, int  channel, const char*, float volume, float attn, int undefined, float pitch);

inline bool IS_DEDICATED_SERVER() {
    return false;
}

static const ::Vector VEC_HULL_MIN{0.f, 0.f, 1.8f};
static const ::Vector VEC_DUCK_HULL_MIN{0.f, 0.f, 1.f};

int POINT_CONTENTS(const ::Vector& vec);

enum HULL {head_hull, human_hull, point_hull};

void TRACE_HULL (const ::Vector& vecMidPoint, const ::Vector& vecTemp, IGNORE_MONSTERS ignore_monsters, HULL hull, edict_t* pEdict, TraceResult* tr);

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

int COMPARE_FILE_TIME(const std::string& filename1, const std::string& filename2, int* iCompare);

void SERVER_COMMAND(const char* cmd);

inline void SET_ORIGIN (edict_t* edict, const ::Vector& v) {
    edict->v.origin = v;
}
