#include "Project_Nodes_Onto_Ways.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Way_Bvh.hpp>

using namespace Mlib;

void Mlib::project_nodes_onto_ways(
    std::map<std::string, Node>& nodes,
    const std::list<FixedArray<CompressedScenePos, 2, 2>>& way_segments,
    double scale)
{
    WayBvh way_bvh{ way_segments };
    for (auto& [node_id, node] : nodes) {
        node.position = way_bvh.project_onto_way(node_id, node, scale);
    }
}
