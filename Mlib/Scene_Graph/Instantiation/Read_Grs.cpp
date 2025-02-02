#include "Read_Grs.hpp"
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <iterator>

using namespace Mlib;
using namespace Mlib::Grs;

struct String16 {
    char c[16];
};
static_assert(sizeof(String16) == 16);

struct ResourceGroupBuffer {
    String16 elements[8];
    uint8_t reserved[8];
    char name[16];
};
static_assert(sizeof(ResourceGroupBuffer) == 8 * 16 + 8 + 16);

struct ResourceGroupBuffers {
    ResourceGroupBuffer types[8];
};
static_assert(sizeof(ResourceGroupBuffers) == 8 * sizeof(ResourceGroupBuffer));

Model Mlib::Grs::load_grs(std::istream& istr, IoVerbosity verbosity) {
    Model result;
    if (auto magic7 = read_binary<uint32_t>(istr, "magic7", verbosity); magic7 != 7) {
        THROW_OR_ABORT("Invalid magic number 7 in grs-file: " + std::to_string(magic7));
    }
    auto magic68 = read_binary<uint32_t>(istr, "magic6/8", verbosity);
    if ((magic68 != 6) && (magic68 != 8)) {
        THROW_OR_ABORT("Invalid magic number 6 or 8 in grs-file: " + std::to_string(magic68));
    }
    seek_relative_positive(istr, 2408, verbosity);
    auto resource_group_buffers = read_binary<ResourceGroupBuffers>(istr, "resource groups", verbosity);
    result.resource_groups.reserve(std::size(resource_group_buffers.types));
    for (const auto& group_buffer : resource_group_buffers.types) {
        auto name = std::string(std::string(group_buffer.name, std::size(group_buffer.name)).c_str());
        if (name.empty()) {
            break;
        }
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Resource group: \"" << name << '"';
        }
        auto& group = result.resource_groups.emplace_back();
        group.name = name;
        group.elements.reserve(std::size(group_buffer.elements));
        for (const auto& element_buffer : group_buffer.elements) {
            auto element_name = std::string(std::string(element_buffer.c, std::size(element_buffer.c)).c_str());
            if (element_name.empty()) {
                break;
            }
            if (any(verbosity & IoVerbosity::METADATA)) {
                linfo() << "Resource: \"" << element_name << '"';
            }
            group.elements.emplace_back(std::move(element_name));
        }
    }
    if (auto magic68_2 = read_binary<uint32_t>(istr, "magic6/8 2", verbosity); magic68_2 != magic68) {
        THROW_OR_ABORT(std::format(
            "Invalid repeated magic number 6 or 8 in grs-file. "
            "First: {}, second: {}", magic68, magic68_2));
    }
    {
        auto cell0_address = read_binary<uint32_t>(istr, "address of cell 0", verbosity);
        auto current_positon = istr.tellg();
        if (cell0_address < current_positon) {
            THROW_OR_ABORT("Address of cell 0 is behind current read position");
        }
        seek_relative_positive(istr, cell0_address - current_positon, verbosity);
    }
    std::list<Cell> cells;
    while (istr.peek() != EOF) {
        auto size16 = read_binary<uint16_t>(istr, "size 16", verbosity);
        auto size8 = read_binary<uint16_t>(istr, "size 8", verbosity);
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Size 16: " << size16;
            linfo() << "Size 8: " << size8;
        }
        seek_relative_positive(istr, 12, verbosity);
        if (cells.size() > (size_t)1e6) {
            THROW_OR_ABORT("Too many cells");
        }
        auto& cell = cells.emplace_back(uninitialized);
        {
            static_assert(sizeof(FixedArray<float, 3>) == 3 * 4);
            auto min = read_binary<DefaultUnitialized<FixedArray<float, 3>>>(istr, "aabb min", verbosity);
            auto max = read_binary<DefaultUnitialized<FixedArray<float, 3>>>(istr, "aabb max", verbosity);
            cell.aabb = AxisAlignedBoundingBox<float, 3>::from_min_max(min, max);
        }
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "AABB: " << cell.aabb;
        }
        cell.coords16.resize(size16);
        read_vector(istr, cell.coords16, "coords 16", verbosity);
        cell.coords8.resize(size8);
        read_vector(istr, cell.coords8, "coords 8", verbosity);
        if (any(verbosity & IoVerbosity::METADATA)) {
            for (const auto& e : cell.coords16) {
                linfo() << "Position 16: " << e.p << " - 0x" << std::hex << e.flags;
            }
            for (const auto& e : cell.coords8) {
                linfo() << "Position 8: " << e.p.casted<uint16_t>() << " - 0x" << std::hex << +e.flags;
            }
        }
        if (istr.peek() == EOF) {
            break;
        }
        // Seek forward until aligned to next 16 bit address.
        auto current_positon = istr.tellg();
        auto alignment = (std::streampos)0xF;
        auto next_position = (current_positon + alignment) & (~alignment);
        if (next_position < current_positon) {
            THROW_OR_ABORT("Seekg integer overflow");
        }
        if (any(verbosity & IoVerbosity::METADATA)) {
            linfo() << "Seek 0x" << std::hex << current_positon << " -> 0x" << std::hex << next_position;
        }
        seek_relative_positive(istr, next_position - current_positon, verbosity);
    }
    result.cells = { cells.begin(), cells.end() };
    return result;
}

Model Mlib::Grs::load_grs(const std::string& filename, IoVerbosity verbosity) {
    auto f = create_ifstream(filename, std::ios::binary);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open \"" + filename + '"');
    }
    try {
        return load_grs(*f, verbosity);
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Error reading from file \"" + filename + "\": " + e.what());
    }
}

// ImHex pattern
//
// struct Header {
//     u32 magic7;
//     u32 magic8;
//     u8 reserved[2408];
// };
//
// struct String16 {
//     char c[16];
// };
//
// struct GrassType {
//     String16 elements[8];
//     u8 reserved[8];
//     char type[16];
// };
//
// struct GrassTypes {
//     GrassType types[8];
// };
//
// struct Header2 {
//     u32 magic8;
//     u32 cell0_address;
// };
//
// struct Coords16 {
//     u16 p[3];
//     u16 reserved;
// };
//
// struct Coords8 {
//     u8 p[3];
//     u8 reserved;
// };
//
// struct Cell {
//     u16 size0;
//     u16 size1;
//     u8 zeros[12];
//     float min[3];
//     float max[3];
//     Coords16 coords16[size0];
//     Coords8 coords8[size1];
// };
//
// Header header @ 0x0;
// GrassTypes grass_types @ 0x970;
// Header2 header2 @ 0xe30;
// Cell cella @ 0x00b9c0;
// Cell cell0 @ 0x2db690;
// Cell cell1 @ 0x2db9a0;
// Cell cell2 @ 0x2dbaf0;
// Cell cell3 @ 0x2dbc60;
// Cell cell4 @ 0x2dbd80;
// Cell cell5 @ 0xad30e0;
