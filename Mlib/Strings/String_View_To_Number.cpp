#include "String_View_To_Number.hpp"
#include <cmath>
#include <stdexcept>

using namespace Mlib;

double Mlib::safe_stod(const std::string_view& s) {
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

float Mlib::safe_stof(const std::string_view& s) {
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

int Mlib::safe_stoi(const std::string_view& s) {
    return safe_stox<int>(s, "int");
}

unsigned int Mlib::safe_stou(const std::string_view& s) {
    return safe_stox<unsigned int>(s, "uint");
}

uint16_t Mlib::safe_stou16(const std::string_view& s) {
    return safe_stox<uint16_t>(s, "uint16");
}

uint64_t Mlib::safe_stou64(const std::string_view& s) {
    return safe_stox<uint64_t>(s, "uint64");
}

size_t Mlib::safe_stoz(const std::string_view& s) {
    return safe_stox<size_t>(s, "size_t");
}

bool Mlib::safe_stob(const std::string_view& s) {
    if (s == "0") {
        return false;
    }
    if (s == "1") {
        return true;
    }
    throw std::runtime_error("Could not convert \"" + std::string{s} + "\" to bool");
}
