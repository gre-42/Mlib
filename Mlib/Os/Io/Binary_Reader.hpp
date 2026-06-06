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
    inline std::string read_string(std::string_view message) {
        auto len = Mlib::read_binary<LengthType>(istr_, message, verbosity_);
        return Mlib::read_string(istr_, len, message, verbosity_);
    }
    template <class T>
    T read_binary(std::string_view message) {
        return Mlib::read_binary<T>(istr_, message, verbosity_);
    }
    template <class TData>
    void read_vector(const std::span<TData>& vec, std::string_view message) {
        Mlib::read_vector(istr_, vec, message, verbosity_);
    }
    template <class TVec>
    void read_vector(TVec& vec, std::string_view message) {
        Mlib::read_vector(istr_, vec, message, verbosity_);
    }
    inline std::vector<std::byte> read_all_vector(std::string_view message) {
        return Mlib::read_all_vector(istr_, message, verbosity_);
    }
    inline void seek_relative_positive(std::streamoff amount) {
        Mlib::seek_relative_positive(istr_, amount, verbosity_);
    }

private:
    std::istream& istr_;
    IoVerbosity verbosity_;
};

}
