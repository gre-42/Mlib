#include "Apply_Heightmap.hpp"
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation/Transformation_Matrix.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Height_Sampler.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Height_Binding.hpp>
#include <Mlib/Render/Renderables/Triangle_Sampler/Terrain_Type.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

struct NodeHeight {
    double height;
    double smooth_height;
    double layer;
};

struct NeighborWeight {
    std::string id;
    double weight;
    int layer;
    double bridge_height;
};

}

using namespace Mlib;

void Mlib::apply_heightmap(
    const TerrainTypeTriangleList& tl_terrain,
    const std::map<EntranceType, std::set<OrderableFixedArray<CompressedScenePos, 2>>>& entrances,
    CompressedScenePos tunnel_height,
    CompressedScenePos extrude_air_support_amount,
    std::list<FixedArray<CompressedScenePos, 3>*>& in_vertices,
    std::set<const FixedArray<CompressedScenePos, 3>*>& vertices_to_delete,
    const HeightSampler& height_sampler,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<CompressedScenePos, 2>, NodeHeightBinding>& node_height_bindings,
    const std::unordered_map<FixedArray<CompressedScenePos, 3>*, VertexHeightBinding<CompressedScenePos>>& vertex_height_bindings,
    float street_node_smoothness,
    size_t street_node_smoothing_iterations,
    const Interp<double>& layer_heights)
{
    // Smoothen raw 2D street nodes, ignoring which triangles they contributed to.
    std::map<std::string, NodeHeight> node_height;
    if (street_node_smoothness != 0) {
        // Find all node neighbors and compute a weight for each
        // neighbor based on the distance.
        std::map<std::string, std::list<NeighborWeight>> node_neighbors;
        for (const auto& w : ways) {
            auto layer_it = w.second.tags.find("layer");
            int layer = (layer_it == w.second.tags.end()) ? 0 : safe_stoi(layer_it->second);
            if ((layer != 0) && !layer_heights.is_within_range((double)layer)) {
                continue;
            }
            double bridge_height = parse_meters(w.second.tags, "bridge_height", NAN);
            bool ref_is_ground =
                !std::isnan(bridge_height) &&
                w.second.tags.contains("bridge_height_reference", "ground");
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != w.second.nd.end()) {
                    double bridge_height_ref = bridge_height;
                    if (ref_is_ground) {
                        CompressedScenePos z;
                        if (height_sampler((nodes.at(*it).position + nodes.at(*s).position) / 2, z)) {
                            bridge_height_ref += (double)z;
                        } else {
                            lerr() << "Bridge with ref=ground is not inside heightmap. Way ID: " << w.first;
                        }
                    }
                    if (all(nodes.at(*it).position == nodes.at(*s).position)) {
                        THROW_OR_ABORT("Duplicates in neighboring points: " + *it + " - " + *s);
                    }
                    double weight = 1 / std::sqrt(sum(squared(nodes.at(*it).position - nodes.at(*s).position)));
                    node_neighbors[*s].push_back({.id = *it, .weight = weight, .layer = layer, .bridge_height = bridge_height_ref});
                    node_neighbors[*it].push_back({.id = *s, .weight = weight, .layer = layer, .bridge_height = bridge_height_ref});
                }
            }
        }
        // Iterate over the nodes with at least one neighbor
        // and compute their initial heights.
        for (const auto& n : node_neighbors) {
            double layer = 0;
            for (const auto& nn : n.second) {
                layer += (double)nn.layer;
            }
            layer /= (double)n.second.size();
            size_t nbridge_heights = 0;
            double bridge_height = 0;
            for (const auto& nn : n.second) {
                if (!std::isnan(nn.bridge_height)) {
                    bridge_height += nn.bridge_height;
                    ++nbridge_heights;
                }
            }
            if (nbridge_heights != 0) {
                bridge_height /= (double)nbridge_heights;
            }
            if (nbridge_heights != 0) {
                node_height[n.first] = {
                    .height = layer_heights(layer) + bridge_height - layer_heights(0),
                    .smooth_height = layer_heights(layer) + bridge_height - layer_heights(0)};
            } else {
                if (layer == 0) {
                    // If the ways to all neighbors are on the ground (or they cancel out to 0),
                    // pick the height of the heightmap exactly on the node.
                    CompressedScenePos z;
                    if (height_sampler(nodes.at(n.first).position, z)) {
                        node_height[n.first] = {
                            .height = (double)z,
                            .smooth_height = (double)z};
                    }
                } else {
                    // If some ways are not on the ground, and the heights don't cancel out to 0,
                    // interpolate the height using the "layer_heights" interpolator.
                    node_height[n.first] = {
                        .height = layer_heights(layer),
                        .smooth_height = layer_heights(layer)};
                }
            }
        }
        for (auto nnit = node_neighbors.begin(); nnit != node_neighbors.end();) {
            auto nit = nodes.find(nnit->first);
            if (auto tit = nit->second.tags.find("smoothing"); (tit != nit->second.tags.end()) && (!safe_stob(tit->second))) {
                nnit = node_neighbors.erase(nnit);
            } else {
                ++nnit;
            }
        }
        // Smoothen the heights.
        for (size_t i = 0; i < street_node_smoothing_iterations; ++i) {
            for (const auto& n : node_neighbors) {
                auto hit = node_height.find(n.first);
                if (hit != node_height.end()) {
                    double mean_height = 0;
                    double sum_weights = 0;
                    for (const auto& b : n.second) {
                        auto it = node_height.find(b.id);
                        if (it != node_height.end()) {
                            mean_height += b.weight * it->second.smooth_height;
                            sum_weights += b.weight;
                        }
                    }
                    if (sum_weights > 0) {
                        mean_height /= sum_weights;
                        hit->second.smooth_height = street_node_smoothness * mean_height + (1 - street_node_smoothness) * hit->second.height;
                    }
                }
            }
        }
    }
    std::map<EntranceType, std::set<const FixedArray<CompressedScenePos, 3>*>> terrain_entrance_vertices;
    for (const auto& [_, tt] : tl_terrain.map()) {
        for (const auto& t : tt->triangles) {
            for (const auto& v : t.flat_iterable()) {
                OrderableFixedArray<CompressedScenePos, 2> vc{v.position(0), v.position(1)};
                for (const auto& e : entrances) {
                    if (e.second.contains(vc)) {
                        terrain_entrance_vertices[e.first].insert(&v.position);
                    }
                }
            }
        }
    }
    if (auto filename = getenv("TUNNEL_ENTRANCES_OBJ"); filename != nullptr) {
        std::list<FixedArray<ColoredVertex<CompressedScenePos>, 3>> tcp;
        for (const auto& [_, tt] : tl_terrain.map()) {
            for (const auto& t : tt->triangles) {
                bool found = false;
                for (const auto& v : t.flat_iterable()) {
                    OrderableFixedArray<CompressedScenePos, 2> vc{v.position(0), v.position(1)};
                    if (entrances.at(EntranceType::TUNNEL).contains(vc)) {
                        found = true;
                    }
                }
                if (found) {
                    tcp.push_back(t);
                }
            }
        }
        save_obj(
            filename,
            IndexedFaceSet<float, CompressedScenePos, size_t>{ tcp },
            nullptr);  // material
    }
    // Transfer smoothening of street nodes to the triangles they produced.
    // The mapping node -> triangle vertices is stored in the "node_height_bindings" mapping.
    // Note that the 2D coordinates of OSM nodes are garantueed to be unique to exactly one height.
    // Also, duplicate nodes were already removed while parsing the OSM XML-file.
    std::map<OrderableFixedArray<CompressedScenePos, 2>, std::list<FixedArray<CompressedScenePos, 3>*>> vertex_instances_map;
    for (FixedArray<CompressedScenePos, 3>* iv : in_vertices) {
        OrderableFixedArray<CompressedScenePos, 2> vc = uninitialized;
        auto hit = vertex_height_bindings.find(iv);
        if (hit != vertex_height_bindings.end()) {
            vc = hit->second.value();
        } else {
            vc = OrderableFixedArray<CompressedScenePos, 2>{ (*iv)(0), (*iv)(1) };
        }
        vertex_instances_map[vc].push_back(iv);
    }
    for (auto& position : vertex_instances_map) {
        FixedArray<CompressedScenePos, 2> vc = uninitialized;
        // Try to apply height bindings.
        auto it = node_height_bindings.find(OrderableFixedArray<CompressedScenePos, 2>{position.first(0), position.first(1)});
        if (it != node_height_bindings.end()) {
            // Note that node_height is empty if street_node_smoothness == 0,
            // so this test will then always return false.
            if (auto hit = node_height.find(it->second.str()); hit != node_height.end()) {
                for (auto& pc : position.second) {
                    (*pc)(2) += (CompressedScenePos)(hit->second.smooth_height * scale);
                    // Both the tunnel and the street vertices are part of the in_vertices.
                    // The terrain vertices lying on the tunnel vertices are therefore
                    // first moving down with the tunnel vertices in the line above,
                    // and then moved up by the line below.
                    if (terrain_entrance_vertices[EntranceType::TUNNEL].contains(pc)) {
                        (*pc)(2) += tunnel_height * scale;
                    }
                    if (terrain_entrance_vertices[EntranceType::BRIDGE].contains(pc)) {
                        (*pc)(2) += extrude_air_support_amount * scale;
                    }
                }
                continue;
            }
            vc = nodes.at(it->second.str()).position;
        } else {
            vc = {position.first(0), position.first(1)};
        }
        // If no height binding could be applied, use the raw heightmap value.
        CompressedScenePos z;
        if (!height_sampler(vc, z)) {
            // lerr() << "Height out of bounds.";
            for (auto& pc : position.second) {
                if (!vertices_to_delete.insert(pc).second) {
                    THROW_OR_ABORT("Could not insert vertex to delete");
                }
            }
        } else {
            for (auto& pc : position.second) {
                (*pc)(2) += z * scale;
            }
        }
    }
}
