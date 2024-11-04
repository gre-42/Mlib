#include "Dds_Info.hpp"
#include <Mlib/Images/Dds_Header.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cstdint>
#include <sstream>

using namespace Mlib;

DdsInfo DdsInfo::load_from_file(const std::string& filename) {
    auto ifstr = create_ifstream(filename, std::ios_base::binary);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + "'");
    }
    try {
        return load_from_stream(*ifstr);
    } catch (const std::exception& e) {
        throw std::runtime_error("Could not load \"" + filename + "\": " + e.what());
    }
}

DdsInfo DdsInfo::load_from_stream(std::istream& istream) {
    uint32_t actual_dds_magic;
    istream.read((char*)&actual_dds_magic, 4);
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read DDS magic key");
    }
    if (actual_dds_magic != DDS_MAGIC) {
        THROW_OR_ABORT("Invalid DDS magic key");
    }
    DdsHeader header;
    istream.read((char*)&header, sizeof(header));
    if (istream.fail()) {
        THROW_OR_ABORT("Could not read DDS header");
    }
    // DdsImage result;
    // result.resize(header.width, header.height);
    return DdsInfo{
        .width = integral_cast<int>(header.width),
        .height = integral_cast<int>(header.height)};
}

DdsInfo DdsInfo::load_from_buffer(const std::vector<std::byte>& buffer) {
    if (buffer.size() < sizeof(DdsHeader) + sizeof(uint32_t)) {
        THROW_OR_ABORT("DDS buffer too small");
    }
    std::stringstream sstr;
    for (size_t i = 0; i < sizeof(DdsHeader) + sizeof(uint32_t); ++i) {
        sstr << (char)buffer[i];
    }
    return load_from_stream(sstr);
}
