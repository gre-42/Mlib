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
