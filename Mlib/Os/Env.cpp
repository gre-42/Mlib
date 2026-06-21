#include "Env.hpp"
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <cstdlib>
#include <filesystem>
#include <mutex>
#include <stdexcept>

using namespace Mlib;

static std::string g_app_reldir;

std::optional<std::string> Mlib::try_getenv(const char* name) {
    static std::mutex mutex;
    std::scoped_lock lock{mutex};
    const char* result = std::getenv(name);
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

std::string Mlib::getenv_default(const char* name, const std::string& deflt) {
    auto result = try_getenv(name);
    if (!result.has_value()) {
        return deflt;
    } else {
        return *result;
    }
}

float Mlib::getenv_default_float(const char* n, float deflt) {
    auto v = try_getenv(n);
    if (!v.has_value()) {
        return deflt;
    }
    return Mlib::safe_stof(*v);
}

int Mlib::getenv_default_int(const char* n, int deflt) {
    auto v = try_getenv(n);
    if (!v.has_value()) {
        return deflt;
    }
    return Mlib::safe_stoi(*v);
}

unsigned int Mlib::getenv_default_uint(const char* n, unsigned int deflt) {
    auto v = try_getenv(n);
    if (!v.has_value()) {
        return deflt;
    }
    return Mlib::safe_stou(*v);
}

size_t Mlib::getenv_default_size_t(const char* n, size_t deflt) {
    auto v = try_getenv(n);
    if (!v.has_value()) {
        return deflt;
    }
    return Mlib::safe_stoz(*v);
}

bool Mlib::getenv_default_bool(const char* n, bool deflt) {
    auto v = try_getenv(n);
    if (!v.has_value()) {
        return deflt;
    }
    return Mlib::safe_stob(*v);
}

#if defined(__ANDROID__)
Utf8Path Mlib::get_appdata_directory() {
    return "/";
}
#elif defined(__EMSCRIPTEN__)
Utf8Path Mlib::get_appdata_directory() {
    return "/appdata";
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
    auto parent = try_getenv("HOME");
#elif _WIN32
    auto parent = try_getenv("APPDATA");
#else
    #error Could not determine OS
#endif
    if (!parent.has_value()) {
        throw std::runtime_error("Could not determine home directory");
    }
    return Utf8Path{*parent} / g_app_reldir;
}
#endif

Utf8Path Mlib::get_path_in_appdata_directory(const std::initializer_list<Utf8Path>& child_path) {
    auto res = get_appdata_directory();
    for (const auto& s : child_path) {
        res /= s;
    }
    return res;
}
