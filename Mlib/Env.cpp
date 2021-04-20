#include "Env.hpp"
#include <Mlib/Strings/From_Number.hpp>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

using namespace Mlib;

const char* Mlib::getenv_default(const char* name, const char* deflt) {
    const char* result = getenv(name);
    if (result == nullptr) {
        return deflt;
    } else {
        return result;
    }
}

float Mlib::getenv_default_float(const char* n, float deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stof(v);
}

int Mlib::getenv_default_int(const char* n, int deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stoi(v);
}

size_t Mlib::getenv_default_size_t(const char* n, size_t deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stoz(v);
}

bool Mlib::getenv_default_bool(const char* n, bool deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stob(v);
}

std::string Mlib::get_home_directory() {
#if defined(__linux__) || defined(__APPLE__)
    const char* res = getenv("HOME");
#elif _WIN32
    const char* res = getenv("APPDATA");
#else
    #error Could not determine OS
#endif
    if (res == nullptr) {
        throw std::runtime_error("Could not determine home directory");
    }
    return res;
}

std::string Mlib::get_path_in_home_directory(const std::initializer_list<std::string>& child_path) {
    fs::path res = get_home_directory();
    for (const auto& s : child_path) {
        res /= fs::path(s);
    }
    return res.string();
}
