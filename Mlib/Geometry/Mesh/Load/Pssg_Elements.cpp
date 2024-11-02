#include "Pssg_Elements.hpp"
#include <Mlib/Io/Endian.hpp>

using namespace Mlib;

std::string PssgAttribute::string() const {
    if (data.size() < 4) {
        THROW_OR_ABORT("PSSG string attribute too short");
    }
    auto length = swap_endianness(*reinterpret_cast<const uint32_t*>(data.data()));
    if (length != data.size() - 4) {
        THROW_OR_ABORT("PSSG string attribute size mismatch");
    }
    return std::string((char*)data.data() + 4, data.size() - 4);
}

uint32_t PssgAttribute::uint32() const {
    if (data.size() != 4) {
        THROW_OR_ABORT("PSSG uint32 attribute does not have 4 bytes");
    }
    return swap_endianness(*reinterpret_cast<const uint32_t*>(data.data()));
}

uint64_t PssgAttribute::uint64() const {
    if (data.size() != 8) {
        THROW_OR_ABORT("PSSG uint64 attribute does not have 8 bytes");
    }
    return swap_endianness(*reinterpret_cast<const uint64_t*>(data.data()));
}

float PssgAttribute::float32() const {
    if (data.size() != 4) {
        THROW_OR_ABORT("PSSG float attribute does not have 4 bytes");
    }
    return swap_endianness(*reinterpret_cast<const float*>(data.data()));
}

FixedArray<double, 2> PssgAttribute::dvec2() const {
    if (data.size() != 16) {
        THROW_OR_ABORT("PSSG dvec2 attribute does not have 16 bytes");
    }
    FixedArray<double, 2> result = uninitialized;
    result(0) = swap_endianness(*reinterpret_cast<const double*>(data.data()));
    result(1) = swap_endianness(*reinterpret_cast<const double*>(data.data() + 8));
    return result;
}

std::string PssgNode::pnstring() const {
    auto str = std::string((const char*)data.data(), data.size());
    if (str.empty()) {
        THROW_OR_ABORT("Raw PNSTRING is empty");
    }
    if (str[str.size() - 1] != 0) {
        THROW_OR_ABORT("PNSTRING is not null-terminated");
    }
    return str.substr(0, str.size() - 1);
}
