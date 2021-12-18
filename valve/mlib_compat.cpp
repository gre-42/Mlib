#include "mlib_compat.h"
#include <filesystem>

edict_t* INDEXENT(int index) {
    static std::map<int, edict_t*> map;
    return map[index];
}

int ENTINDEX(const edict_t* e) {
    static std::map<const edict_t*, int> map;
    auto it = map.insert({e, map.size()});
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

void UTIL_ServerPrint(char const*, ...) {
    throw std::runtime_error("Not yet implemented");
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
