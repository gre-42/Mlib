#pragma once
#include <Mlib/Os/Io/Binary.hpp>
#include <concepts>
#include <cstdint>

namespace Mlib {

class BinaryWriter {
public:
    inline explicit BinaryWriter(std::ostream& ostr)
        : ostr_{ ostr }
    {}
    template <std::integral LengthType>
    inline void write_string(const std::string& str, std::string_view message) {
        Mlib::write_binary(ostr_, integral_cast<LengthType>(str.length()), message);
        Mlib::write_iterable(ostr_, str, message);
    }
    template <class T, bool allow_i64 = false>
    inline void write_binary(const T& v, std::string_view message) {
        Mlib::write_binary<T, allow_i64>(ostr_, v, message);
    }
    template <class TIterable>
    void write_iterable(const TIterable& iterable, std::string_view message) {
        Mlib::write_iterable(ostr_, iterable, message);
    }
private:
    std::ostream& ostr_;
};

}
