#pragma once
#include <Mlib/Os/Io/Binary.hpp>
#include <concepts>

namespace Mlib {

class BinaryReader {
public:
    inline BinaryReader(std::istream& istr, IoVerbosity verbosity)
        : istr_{ istr }
        , verbosity_{ verbosity }
    {}
    template <std::integral LengthType>
    inline std::string read_string(const char* message) {
        auto len = Mlib::read_binary<LengthType>(istr_, message, verbosity_);
        return Mlib::read_string(istr_, len, message, verbosity_);
    }
    template <class T>
    T read_binary(const char* message) {
        return Mlib::read_binary<T>(istr_, message, verbosity_);
    }
    template <class TData>
    void read_vector(const std::span<TData>& vec, const char* msg) {
        Mlib::read_vector(istr_, vec, msg, verbosity_);
    }
    template <class TVec>
    void read_vector(TVec& vec, const char* msg) {
        Mlib::read_vector(istr_, vec, msg, verbosity_);
    }
    inline std::vector<std::byte> read_all_vector(const char* msg) {
        return Mlib::read_all_vector(istr_, msg, verbosity_);
    }
    inline void seek_relative_positive(std::streamoff amount) {
        Mlib::seek_relative_positive(istr_, amount, verbosity_);
    }

private:
    std::istream& istr_;
    IoVerbosity verbosity_;
};

}
