#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Map/Map.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

enum class IdeFlags;

struct IdeItem {
    std::string texture_dictionary;
    FixedArray<float, 2> raw_center_distances;
    IdeFlags flags;
    FixedArray<float, 2> center_distances(float radius) const;
};

class DrawDistanceDb {
public:
    DrawDistanceDb();
    ~DrawDistanceDb();
    void add_ide(const std::string& filename);
    const IdeItem& get_item(const std::string& resource_name) const;
private:
    Map<std::string, IdeItem> ide_items_;
};

}