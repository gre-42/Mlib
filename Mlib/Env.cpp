#include "Env.hpp"
#include <Mlib/String.hpp>
#include <cstdlib>

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

bool Mlib::getenv_default_bool(const char* n, bool deflt) {
    const char* v = getenv(n);
    if (v == nullptr) {
        return deflt;
    }
    return Mlib::safe_stob(v);
}
