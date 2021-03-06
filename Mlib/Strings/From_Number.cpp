#include "From_Number.hpp"
#include <sstream>
#include <stdexcept>

using namespace Mlib;

double Mlib::safe_stod(const std::string& s) {
    std::size_t idx;
    double res;
    try {
        res = std::stod(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stod: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stod: \"" + s + '"');
    }
    return res;
}

float Mlib::safe_stof(const std::string& s) {
    std::size_t idx;
    float res;
    try {
        res = std::stof(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stof: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stof: \"" + s + '"');
    }
    return res;
}

int Mlib::safe_stoi(const std::string& s) {
    std::size_t idx;
    int res;
    try {
        res = std::stoi(s, &idx);
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stoi: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stoi: \"" + s + '"');
    }
    return res;
}

unsigned int Mlib::safe_stou(const std::string& s) {
    std::size_t idx;
    unsigned int res;
    try {
        unsigned long ul = std::stoul(s, &idx);
        res = ul;
        if (res != ul) {
            throw std::invalid_argument(s);
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stou: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stou: \"" + s + '"');
    }
    return res;
}

uint64_t Mlib::safe_stou64(const std::string& s) {
    std::size_t idx;
    uint64_t res;
    try {
        unsigned long long ul = std::stoull(s, &idx);
        res = ul;
        if (res != ul) {
            throw std::invalid_argument(s);
        }
    } catch (const std::invalid_argument&) {
        throw std::invalid_argument("safe_stou64: \"" + s + '"');
    }
    if (idx != s.length()) {
        throw std::invalid_argument("safe_stou64: \"" + s + '"');
    }
    return res;
}

size_t Mlib::safe_stoz(const std::string& s) {
    size_t res;
    std::stringstream sstr;
    sstr << s;
    sstr >> res;
    if ((sstr.rdbuf()->in_avail() != 0) || sstr.fail()) {
        throw std::invalid_argument("safe_stoz: \"" + s + '"');
    }
    return res;
}

bool Mlib::safe_stob(const std::string& s) {
    if (s == "0") {
        return false;
    }
    if (s == "1") {
        return true;
    }
    throw std::runtime_error("Could not convert \"" + s + "\" to bool");
}
