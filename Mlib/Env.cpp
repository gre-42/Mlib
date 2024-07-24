#include "Env.hpp"
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

namespace fs = std::filesystem;

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
        THROW_OR_ABORT("No environment variable with name \"" + std::string{ name } + "\" exists");
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

#ifdef __ANDROID__
std::string Mlib::get_appdata_directory() {
    return "/";
}
#else
void Mlib::set_app_reldir(const std::string& app_reldir) {
    if (!g_app_reldir.empty()) {
        THROW_OR_ABORT("App reldir already set");
    }
    if (app_reldir.empty()) {
        THROW_OR_ABORT("Trying to set empty app reldir");
    }
    g_app_reldir = app_reldir;
}

std::string Mlib::get_appdata_directory() {
    if (g_app_reldir.empty()) {
        THROW_OR_ABORT("set_app_reldir not called before get_appdata_directory");
    }
#if defined(__linux__) || defined(__APPLE__)
    const char* parent = getenv("HOME");
#elif _WIN32
    const char* parent = getenv("APPDATA");
#else
    #error Could not determine OS
#endif
    if (parent == nullptr) {
        THROW_OR_ABORT("Could not determine home directory");
    }
    return (fs::path{parent} / fs::path{g_app_reldir}).string();
}
#endif

std::string Mlib::get_path_in_appdata_directory(const std::initializer_list<std::string>& child_path) {
    fs::path res = get_appdata_directory();
    for (const auto& s : child_path) {
        res /= fs::path(s);
    }
    return res.string();
}
