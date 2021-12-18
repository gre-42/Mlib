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
