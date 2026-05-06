#include "Env.hpp"
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

using namespace Mlib;

static std::string g_app_reldir;

std::optional<std::string> Mlib::try_getenv(const char* name) {
    const char* result = getenv(name);
    if (result == nullptr) {
        return std::nullopt;
    } else {
        return result;
    }
}

std::string Mlib::str_getenv(const char* name) {
    auto res = try_getenv(name);
    if (res.has_value()) {
        return *res;
    } else {
        throw std::runtime_error("No environment variable with name \"" + std::string{ name } + "\" exists");
    }
}

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

unsigned int Mlib::getenv_default_uint(const char* n, unsigned int deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stou(v);
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

#if defined(__ANDROID__) || defined(__EMSCRIPTEN__)
Utf8Path Mlib::get_appdata_directory() {
    return "/";
}
#else
void Mlib::set_app_reldir(const Utf8Path& app_reldir) {
    if (!g_app_reldir.empty()) {
        throw std::runtime_error("App reldir already set");
    }
    if (app_reldir.empty()) {
        throw std::runtime_error("Trying to set empty app reldir");
    }
    g_app_reldir = app_reldir;
}

Utf8Path Mlib::get_appdata_directory() {
    if (g_app_reldir.empty()) {
        throw std::runtime_error("set_app_reldir not called before get_appdata_directory");
    }
#if defined(__linux__) || defined(__APPLE__)
    const char* parent = getenv("HOME");
#elif _WIN32
    const char* parent = getenv("APPDATA");
#else
    #error Could not determine OS
#endif
    if (parent == nullptr) {
        throw std::runtime_error("Could not determine home directory");
    }
    return Utf8Path{parent} / g_app_reldir;
}
#endif

Utf8Path Mlib::get_path_in_appdata_directory(const std::initializer_list<Utf8Path>& child_path) {
    auto res = get_appdata_directory();
    for (const auto& s : child_path) {
        res /= s;
    }
    return res;
}
