#include "To_Number.hpp"
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

double Mlib::safe_stod(const std::string& s) {
    if (s == "inf") {
        return INFINITY;
    }
    if (s == "-inf") {
        return -INFINITY;
    }
    if (s == "nan") {
        return NAN;
    }
    return safe_stox<double>(s, "double");
}

float Mlib::safe_stof(const std::string& s) {
    if (s == "inf") {
        return INFINITY;
    }
    if (s == "-inf") {
        return -INFINITY;
    }
    if (s == "nan") {
        return NAN;
    }
    return safe_stox<float>(s, "float");
}

int Mlib::safe_stoi(const std::string& s) {
    return safe_stox<int>(s, "int");
}

unsigned int Mlib::safe_stou(const std::string& s) {
    return safe_stox<unsigned int>(s, "uint");
}

uint64_t Mlib::safe_stou64(const std::string& s) {
    return safe_stox<uint64_t>(s, "uint64");
}

size_t Mlib::safe_stoz(const std::string& s) {
    return safe_stox<size_t>(s, "size_t");
}

bool Mlib::safe_stob(const std::string& s) {
    if (s == "0") {
        return false;
    }
    if (s == "1") {
        return true;
    }
    THROW_OR_ABORT("Could not convert \"" + s + "\" to bool");
}
