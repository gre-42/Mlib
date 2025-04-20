#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Map/Map.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

enum class IdeFlags;

struct IdeItem {
    std::string texture_dictionary;
    AddableStepDistances raw_center_distances;
    IdeFlags flags;
    SquaredStepDistances center_distances2(float radius) const;
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