#pragma once
#include <list>
#include <memory>
#include <set>

namespace Mlib {

class TriangleList;
struct OsmResourceConfig;
struct Material;
template <class TData, size_t... tshape>
class OrderableFixedArray;

struct OsmTriangleLists {
    explicit OsmTriangleLists(
        const OsmResourceConfig& config,
        const Material& tunnel_pipe_material);
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
    std::shared_ptr<TriangleList> tl_air_support;
    std::shared_ptr<TriangleList> tl_tunnel_pipe;
    std::shared_ptr<TriangleList> tl_tunnel_bdry;
    std::shared_ptr<TriangleList> tl_tunnel_entry;
    std::set<OrderableFixedArray<float, 2>> tunnel_entrances;
    void insert(const OsmTriangleLists& other);
    std::list<std::shared_ptr<TriangleList>> tls_street_wo_curb() const;
    std::list<std::shared_ptr<TriangleList>> tls_street() const;
    std::list<std::shared_ptr<TriangleList>> tls_wo_subtraction() const;
    std::list<std::shared_ptr<TriangleList>> tls_all() const;
    std::list<std::shared_ptr<TriangleList>> tls_flat() const;
    std::list<std::shared_ptr<TriangleList>> tls_with_vertex_normals() const;
};

}
