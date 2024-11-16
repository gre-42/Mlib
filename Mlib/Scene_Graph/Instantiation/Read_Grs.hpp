#pragma once
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <cstdint>
#include <iosfwd>

namespace Mlib {

enum class IoVerbosity;

namespace Grs {

struct ResourceGroup {
    std::string name;
    std::vector<std::string> elements;
};

struct Coords16 {
    FixedArray<uint16_t, 3> p = uninitialized;
    uint16_t flags;
};
static_assert(sizeof(Coords16) == 8);

struct Coords8 {
    FixedArray<uint8_t, 3> p = uninitialized;
    uint8_t flags;
};
static_assert(sizeof(Coords8) == 4);

struct Cell {
    AxisAlignedBoundingBox<float, 3> aabb;
    std::vector<Coords16> coords16;
    std::vector<Coords8> coords8;
};

struct Model {
    std::vector<ResourceGroup> resource_groups;
    std::vector<Cell> cells;
};

Model load_grs(std::istream& istr, IoVerbosity verbosity);
Model load_grs(const std::string& filename, IoVerbosity verbosity);

}
}
