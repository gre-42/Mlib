#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <sstream>
#include <string>

namespace Mlib {

double safe_stod(const std::string& s);
float safe_stof(const std::string& s);
int safe_stoi(const std::string& s);
unsigned int safe_stou(const std::string& s);
uint64_t safe_stou64(const std::string& s);
size_t safe_stoz(const std::string& s);
bool safe_stob(const std::string& s);

template <class TData>
TData safe_sto(const std::string& s);

template <> inline double safe_sto<double>(const std::string& s) { return safe_stod(s); }
template <> inline float safe_sto<float>(const std::string& s) { return safe_stof(s); }
template <> inline int safe_sto<int>(const std::string& s) { return safe_stoi(s); }
template <> inline unsigned int safe_sto<unsigned>(const std::string& s) { return safe_stou(s); }
template <> inline uint64_t safe_sto<uint64_t>(const std::string& s) { return safe_stou64(s); }
// Disabled because it is either "unsigned int" or "uint64_t"
// template <> inline size_t safe_sto<size_t>(const std::string& s) { return safe_stoz(s); }
template <> inline bool safe_sto<bool>(const std::string& s) { return safe_stob(s); }

template <class T>
T safe_stox(const std::string& s, const char* msg = "safe_stox") {
    T res;
    std::stringstream sstr;
    sstr << s;
    sstr >> res;
    if ((sstr.rdbuf()->in_avail() != 0) || sstr.fail()) {
        THROW_OR_ABORT(msg + std::string(": \"") + s + '"');
    }
    return res;
}

}
