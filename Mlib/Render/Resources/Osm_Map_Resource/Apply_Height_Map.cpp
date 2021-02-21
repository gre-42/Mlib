#include "Apply_Height_Map.hpp"
#include <Mlib/Geometry/Mesh/Save_Obj.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation_Matrix.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Strings/From_Number.cpp>

namespace Mlib {

struct NodeHeight {
    float height;
    float smooth_height;
    float layer;
};

struct NeighborWeight {
    std::string id;
    float weight;
    int layer;
    float bridge_height;
};

}

using namespace Mlib;

void Mlib::apply_height_map(
    const TriangleList& tl_terrain,
    const std::set<OrderableFixedArray<float, 2>>& tunnel_entrances,
    float tunnel_height,
    std::list<FixedArray<float, 3>*>& in_vertices,
    std::set<const FixedArray<float, 3>*>& vertices_to_delete,
    const Array<float>& heightmap,
    const TransformationMatrix<float, 2>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    float street_node_smoothness,
    const Interp<float>& layer_height)
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
            if ((layer != 0) && !layer_height.is_within_range(layer)) {
                continue;
            }
            float bridge_height = NAN;
            auto bridge_height_it = w.second.tags.find("bridge_height");
            if (bridge_height_it != w.second.tags.end()) {
                bridge_height = safe_stof(bridge_height_it->second);
            }
            bool ref_is_ground =
                !std::isnan(bridge_height) &&
                (w.second.tags.find("bridge_height_reference") != w.second.tags.end()) &&
                w.second.tags.at("bridge_height_reference") == "ground";
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != w.second.nd.end()) {
                    float bridge_height_ref = bridge_height;
                    if (ref_is_ground) {
                        FixedArray<float, 2> p = normalization_matrix.transform((nodes.at(*it).position + nodes.at(*s).position) / 2.f);
                        float z;
                        if (bilinear_grayscale_interpolation((1 - p(1)) * (heightmap.shape(0) - 1), p(0) * (heightmap.shape(1) - 1), heightmap, z)) {
                            bridge_height_ref += z;
                        } else {
                            std::cerr << "Bridge with ref=ground is not inside heightmap. Way ID: " << w.first << std::endl;
                        }
                    }
                    float weight = 1 / std::sqrt(sum(squared(nodes.at(*it).position - nodes.at(*s).position)));
                    node_neighbors[*s].push_back({.id = *it, .weight = weight, .layer = layer, .bridge_height = bridge_height_ref});
                    node_neighbors[*it].push_back({.id = *s, .weight = weight, .layer = layer, .bridge_height = bridge_height_ref});
                }
            }
        }
        // Iterate over the nodes with at least one neighbor
        // and compute its initial height.
        for (const auto& n : node_neighbors) {
            float layer = 0;
            for (const auto& nn : n.second) {
                layer += nn.layer;
            }
            layer /= n.second.size();
            size_t nbridge_heights = 0;
            float bridge_height = 0;
            for (const auto& nn : n.second) {
                if (!std::isnan(nn.bridge_height)) {
                    bridge_height += nn.bridge_height;
                    ++nbridge_heights;
                }
            }
            if (nbridge_heights != 0) {
                bridge_height /= nbridge_heights;
            }
            if (nbridge_heights != 0) {
                node_height[n.first] = {
                    .height = layer_height(layer) + bridge_height - layer_height(0),
                    .smooth_height = layer_height(layer) + bridge_height - layer_height(0)};
            } else {
                if (layer == 0) {
                    // If the ways to all neighbors are on the ground (or they cancel out to 0),
                    // pick the height of the heightmap exactly on the node.
                    FixedArray<float, 2> p = normalization_matrix.transform(nodes.at(n.first).position);
                    float z;
                    if (bilinear_grayscale_interpolation((1 - p(1)) * (heightmap.shape(0) - 1), p(0) * (heightmap.shape(1) - 1), heightmap, z)) {
                        node_height[n.first] = {
                            .height = z,
                            .smooth_height = z};
                    }
                } else {
                    // If some ways are not on the ground, and the heights don't cancel out to 0,
                    // interpolate the height using the "layer_height" interpolator.
                    node_height[n.first] = {
                        .height = layer_height(layer),
                        .smooth_height = layer_height(layer)};
                }
            }
        }
        // Smoothen the heights.
        for (size_t i = 0; i < 50; ++i) {
            for (const auto& n : node_neighbors) {
                auto hit = node_height.find(n.first);
                if (hit != node_height.end()) {
                    float mean_height = 0;
                    float sum_weights = 0;
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
    std::set<const FixedArray<float, 3>*> terrain_entrance_vertices;
    for (const auto& t : tl_terrain.triangles_) {
        for (const auto& v : t.flat_iterable()) {
            OrderableFixedArray<float, 2> vc{(v.position)(0), (v.position)(1)};
            if (tunnel_entrances.contains(vc)) {
                terrain_entrance_vertices.insert(&v.position);
            }
        }
    }
    if (true) {
        std::list<FixedArray<ColoredVertex, 3>> tcp;
        for (const auto& t : tl_terrain.triangles_) {
            bool found = false;
            for (const auto& v : t.flat_iterable()) {
                OrderableFixedArray<float, 2> vc{(v.position)(0), (v.position)(1)};
                if (tunnel_entrances.contains(vc)) {
                    found = true;
                }
            }
            if (found) {
                tcp.push_back(t);
            }
        }
        save_obj("/tmp/terrain_entraces.obj", IndexedFaceSet<float, size_t>{tcp});
    }
    // Transfer smoothening of street nodes to the triangles they produced.
    // The mapping node -> triangle vertices is stored in the "height_bindings" mapping.
    // Note that the 2D coordinates of OSM nodes are garantueed to be unique to exactly one height.
    // Also, duplicate nodes were already removed while parsing the OSM XML-file.
    std::map<OrderableFixedArray<float, 2>, std::list<FixedArray<float, 3>*>> vertex_instances_map;
    for (FixedArray<float, 3>* iv : in_vertices) {
        vertex_instances_map[{(*iv)(0), (*iv)(1)}].push_back(iv);
    }
    for (auto& position : vertex_instances_map) {
        FixedArray<float, 2> vc;
        // Try to apply height bindings.
        auto it = height_bindings.find(OrderableFixedArray<float, 2>{position.first(0), position.first(1)});
        if ((it != height_bindings.end()) && (it->second.size() == 1)) {
            // Note that node_height is empty if street_node_smoothness == 0,
            // so this test will then always return false.
            if (auto hit = node_height.find(*it->second.begin()); hit != node_height.end()) {
                for (auto& pc : position.second) {
                    (*pc)(2) += hit->second.smooth_height * scale;
                    if (terrain_entrance_vertices.contains(pc)) {
                        (*pc)(2) += tunnel_height * scale;
                    }
                }
                continue;
            }
            vc = nodes.at(*it->second.begin()).position;
        } else {
            vc = {position.first(0), position.first(1)};
        }
        // If no height binding could be applied, use the raw heightmap value.
        FixedArray<float, 2> p = normalization_matrix.transform(vc);
        float z;
        if (!bilinear_grayscale_interpolation((1 - p(1)) * (heightmap.shape(0) - 1), p(0) * (heightmap.shape(1) - 1), heightmap, z)) {
            // std::cerr << "Height out of bounds." << std::endl;
            for (auto& pc : position.second) {
                if (!vertices_to_delete.insert(pc).second) {
                    throw std::runtime_error("Could not insert vertex to delete");
                }
            }
        } else {
            for (auto& pc : position.second) {
                (*pc)(2) += z * scale;
            }
        }
    }
}
