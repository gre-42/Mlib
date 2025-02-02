#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <charconv>
#include <cstdint>
#include <string_view>

#ifdef __clang__
#include <fast_double_parser/fast_double_parser.hpp>
#endif

namespace Mlib {

double safe_stod(const std::string_view& s);
float safe_stof(const std::string_view& s);
int safe_stoi(const std::string_view& s);
unsigned int safe_stou(const std::string_view& s);
uint64_t safe_stou64(const std::string_view& s);
size_t safe_stoz(const std::string_view& s);
bool safe_stob(const std::string_view& s);

template <class TData>
TData safe_sto(const std::string_view& s);

template <> inline double safe_sto<double>(const std::string_view& s) { return safe_stod(s); }
template <> inline float safe_sto<float>(const std::string_view& s) { return safe_stof(s); }
template <> inline int safe_sto<int>(const std::string_view& s) { return safe_stoi(s); }
template <> inline unsigned int safe_sto<unsigned>(const std::string_view& s) { return safe_stou(s); }
template <> inline uint64_t safe_sto<uint64_t>(const std::string_view& s) { return safe_stou64(s); }
// Disabled because it is either "unsigned int" or "uint64_t"
// template <> inline size_t safe_sto<size_t>(const std::string_view& s) { return safe_stoz(s); }
template <> inline bool safe_sto<bool>(const std::string_view& s) { return safe_stob(s); }

template <class T>
T safe_stox(const std::string_view& s, const char* msg = "safe_stox") {
    T res;
    auto end = s.data() + s.size();
    auto [ptr, ec] = std::from_chars(s.data(), end, res);

    if ((ec != std::errc()) || (ptr != end)) {
        THROW_OR_ABORT(msg + std::string{": \""} + std::string{s} + '"');
    }
    return res;
}

#ifdef __clang__
template <>
inline double safe_stox<double>(const std::string_view& s, const char* msg) {
    double res;
    auto end = s.data() + s.size();
    auto ptr = fast_double_parser::parse_number(s.data(), end, &res);

    if (ptr != end) {
        THROW_OR_ABORT(msg + std::string{": \""} + std::string{s} + '"');
    }
    return res;
}

template <>
inline float safe_stox<float>(const std::string_view& s, const char* msg) {
    return (float)safe_stox<double>(s, msg);
}
#endif

}
