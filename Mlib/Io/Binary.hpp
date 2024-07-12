#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <istream>
#include <ostream>
#include <span>
#include <string>
#include <vector>

namespace Mlib {

enum class IoVerbosity {
    SILENT,
    VERBOSE
};

void print_chars(std::span<char> span);

template <class T>
T read_binary(std::istream& istr, const char* msg, IoVerbosity verbosity) {
    T result;
    istr.read(reinterpret_cast<char*>(&result), sizeof(result));
    if (istr.fail()) {
        THROW_OR_ABORT("Could not read " + std::string(msg) + " from stream");
    }
    if (verbosity == IoVerbosity::VERBOSE) {
        char* begin = reinterpret_cast<char*>(&result);
        char* end = begin + sizeof(result);
        print_chars({ begin, end });
    }
    return result;
}

template <class TData>
void read_vector(std::istream& istr, const std::span<TData>& vec, const char* msg, IoVerbosity verbosity) {
    read_vector(istr, const_cast<std::span<TData>&>(vec), msg, verbosity);
}

template <class TVec>
void read_vector(std::istream& istr, TVec& vec, const char* msg, IoVerbosity verbosity) {
    istr.read(reinterpret_cast<char*>(vec.data()), integral_cast<std::streamsize>(sizeof(typename TVec::value_type) * vec.size()));
    if (istr.fail()) {
        THROW_OR_ABORT("Could not read vector from stream: " + std::string(msg));
    }
    if (verbosity == IoVerbosity::VERBOSE) {
        char* begin = reinterpret_cast<char*>(vec.data());
        char* end = begin + sizeof(vec[0]) * vec.size();
        print_chars({ begin, end });
    }
}

std::string read_string(std::istream& istr, size_t length, const char* msg, IoVerbosity verbosity);

void seek_relative_positive(std::istream& str, std::streamoff amount, IoVerbosity verbosity);

template <class T>
void write_binary(std::ostream& ostr, const T& value, const char* msg) {
    ostr.write((const char*)&value, sizeof(value));
    if (ostr.fail()) {
        THROW_OR_ABORT("Could not write " + std::string(msg) + "to stream");
    }
}

}
