#pragma once
#include <list>
#include <memory>

namespace Mlib {

class TriangleList;
struct OsmResourceConfig;

struct OsmTriangleLists {
    explicit OsmTriangleLists(const OsmResourceConfig& config);
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
    std::shared_ptr<TriangleList> tl_air_curb_street;
    std::shared_ptr<TriangleList> tl_air_curb_path;
    void insert(const OsmTriangleLists& other);
    std::list<std::shared_ptr<TriangleList>> tls_street_wo_curb() const;
    std::list<std::shared_ptr<TriangleList>> tls_street() const;
    std::list<std::shared_ptr<TriangleList>> tls_ground() const;
    std::list<std::shared_ptr<TriangleList>> tls_ground_wo_curb() const;
};

}
