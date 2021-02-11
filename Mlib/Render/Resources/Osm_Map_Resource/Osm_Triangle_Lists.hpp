#pragma once
#include <memory>

namespace Mlib {

class TriangleList;
struct OsmMapResourceConfig;

struct OsmTriangleLists {
    explicit OsmTriangleLists(const OsmMapResourceConfig& config);
    ~OsmTriangleLists();
    std::shared_ptr<TriangleList> tl_terrain;
    std::shared_ptr<TriangleList> tl_terrain_visuals;
    std::shared_ptr<TriangleList> tl_terrain_street_extrusion;
    std::shared_ptr<TriangleList> tl_street_crossing;
    std::shared_ptr<TriangleList> tl_path_crossing;
    std::shared_ptr<TriangleList> tl_street;
    std::shared_ptr<TriangleList> tl_path;
    std::shared_ptr<TriangleList> tl_curb_street;
    std::shared_ptr<TriangleList> tl_curb_path;
    std::shared_ptr<TriangleList> tl_curb2_street;
    std::shared_ptr<TriangleList> tl_curb2_path;
};

}
