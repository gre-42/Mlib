#pragma once
#include <Mlib/Io/Binary.hpp>
#include <cstdint>

namespace Mlib {

class BinaryWriter {
public:
    inline explicit BinaryWriter(std::ostream& ostr)
        : ostr_{ ostr }
    {}
    inline void write_string(const std::string& str, const char* message) {
        Mlib::write_binary(ostr_, integral_cast<uint32_t>(str.length()), message);
        Mlib::write_iterable(ostr_, str, message);
    }
    template <class T>
    inline void write_binary(const T& v, const char* message) {
        return Mlib::write_binary(ostr_, v, message);
    }
private:
    std::ostream& ostr_;
};

}
