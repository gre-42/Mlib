#pragma once
#include <Mlib/Io/Binary.hpp>

namespace Mlib {

class BinaryReader {
public:
    inline BinaryReader(std::istream& istr, IoVerbosity verbosity)
        : istr_{ istr }
        , verbosity_{ verbosity }
    {}
    inline std::string read_string(const char* message) {
        auto len = Mlib::read_binary<uint32_t>(istr_, message, verbosity_);
        return Mlib::read_string(istr_, len, message, verbosity_);
    }
    template <class T>
    inline T read_binary(const char* message) {
        return Mlib::read_binary<T>(istr_, message, verbosity_);
    }
    void seek_relative_positive(std::streamoff amount) {
        Mlib::seek_relative_positive(istr_, amount, verbosity_);
    }

private:
    std::istream& istr_;
    IoVerbosity verbosity_;
};

}
