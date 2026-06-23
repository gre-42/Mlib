#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <istream>
#include <ostream>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Mlib {

enum class IoVerbosity {
    SILENT = 0,
    DATA = 1 << 0,
    METADATA = 1 << 1
};

inline IoVerbosity operator & (IoVerbosity a, IoVerbosity b) {
    return (IoVerbosity)((int)a & (int)b);
}

inline IoVerbosity operator | (IoVerbosity a, IoVerbosity b) {
    return (IoVerbosity)((int)a | (int)b);
}

inline IoVerbosity& operator |= (IoVerbosity& a, IoVerbosity b) {
    (int&)a |= (int)b;
    return a;
}

inline bool any(IoVerbosity v) {
    return v != IoVerbosity::SILENT;
}

void print_char(char c);
void print_chars(std::span<char> span, std::string_view message = "");

template <class T, bool allow_i64 = false>
T read_binary(std::istream& istr, std::string_view message, IoVerbosity verbosity) {
    static_assert(allow_i64 || (!std::is_same_v<T, int64_t> && !std::is_same_v<T, uint64_t>),
                  "64-bit integers are not allowed unless allow_i64 is true.");
    T result;
    istr.read(reinterpret_cast<char*>(&result), sizeof(result));
    if (istr.fail()) {
        throw std::runtime_error("Could not read " + std::string(message) + " from stream");
    }
    if (any(verbosity & IoVerbosity::DATA)) {
        char* begin = reinterpret_cast<char*>(&result);
        char* end = begin + sizeof(result);
        print_chars({ begin, end }, message);
    }
    return result;
}

template <class TData>
void read_vector(std::istream& istr, const std::span<TData>& vec, std::string_view message, IoVerbosity verbosity) {
    read_vector(istr, const_cast<std::span<TData>&>(vec), message, verbosity);
}

template <class TVec>
void read_vector(std::istream& istr, TVec& vec, std::string_view message, IoVerbosity verbosity) {
    istr.read(reinterpret_cast<char*>(vec.data()), integral_cast<std::streamsize>(sizeof(typename TVec::value_type) * vec.size()));
    if (istr.fail()) {
        throw std::runtime_error("Could not read vector from stream: " + std::string(message));
    }
    if (any(verbosity & IoVerbosity::DATA)) {
        char* begin = reinterpret_cast<char*>(vec.data());
        char* end = begin + sizeof(vec[0]) * vec.size();
        print_chars({ begin, end }, message);
    }
}

std::vector<std::byte> read_all_vector(std::istream& istr, std::string_view message, IoVerbosity verbosity);

std::string read_string(std::istream& istr, size_t length, std::string_view message, IoVerbosity verbosity);

void seek_relative_positive(std::istream& str, std::streamoff amount, IoVerbosity verbosity);

template <class T, bool allow_i64 = false>
void write_binary(std::ostream& ostr, const T& value, std::string_view message) {
    static_assert(allow_i64 || (!std::is_same_v<T, int64_t> && !std::is_same_v<T, uint64_t>),
                  "64-bit integers are not allowed unless allow_i64 is true.");
    ostr.write((const char*)&value, sizeof(value));
    if (ostr.fail()) {
        throw std::runtime_error("Could not write " + std::string(message) + "to stream");
    }
}

template <class TIterable>
void write_iterable(std::ostream& ostr, const TIterable& iterable, std::string_view message) {
    for (const auto& e : iterable) {
        write_binary(ostr, e, message);
    }
}

}
