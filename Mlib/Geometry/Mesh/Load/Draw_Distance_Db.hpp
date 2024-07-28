#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Map/Map.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

struct RawIdeItem {
    std::string texture_dictionary;
    FixedArray<float, 2> center_distances;
};

struct IdeItem {
    const std::string& texture_dictionary;
    FixedArray<float, 2> center_distances;
};

class DrawDistanceDb {
public:
    DrawDistanceDb();
    ~DrawDistanceDb();
    void add_ide(const std::string& filename);
    IdeItem get_item(
        const std::string& resource_name,
        float radius) const;
private:
    Map<std::string, RawIdeItem> ide_items_;
};

}