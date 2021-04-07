#include "Osm_Map_Resource_Helpers.hpp"
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Mesh/Triangle_Sampler2.hpp>
#include <Mlib/Geometry/Static_Face_Lightning.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Bounding_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Compute_Area.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Rectangle.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Steiner_Point_Info.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Street_Bvh.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Terrain_Type.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Triangulate_Terrain_Or_Ceilings.hpp>
#include <Mlib/Render/Resources/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <Mlib/Strings/String.hpp>
#include <regex>

using namespace Mlib;

static std::list<FixedArray<float, 2>> smooth_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    float scale,
    float max_width)
{
    std::list<FixedArray<float, 2>> result;
    for (auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            auto p0 = nodes.at(*it).position;
            auto p1 = nodes.at(*s).position;
            float width = std::sqrt(sum(squared(p0 - p1)));
            auto refined = linspace_multipliers<float>(std::max(2, int(width / scale / max_width))).flat_iterable();
            for (auto a = refined.begin(); a != refined.end(); ++a) {
                auto b = a;
                ++b;
                if (b != refined.end() || &*s == &nd.back()) {
                    auto pp0 = a->first * p0 + a->second * p1;
                    result.push_back(pp0);
                }
            }
        }
    }
    return result;
}

// std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> Mlib::gen_steiner_point_map(std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> steiner_point_map;
//     for (auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             throw std::runtime_error("Could not generate steiner point map");
//         }
//     }
//     return steiner_point_map;
// }
// 
// std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> Mlib::gen_const_steiner_point_map(const std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> steiner_point_map;
//     for (auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             throw std::runtime_error("Could not generate const steiner point map");
//         }
//     }
//     return steiner_point_map;
// }

void Mlib::draw_node(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pos2d,
    float size)
{
    ColoredVertex v00{
        .position = FixedArray<float, 3>{pos2d(0) - size, pos2d(1) - size, 0.f},
        .color = FixedArray<float, 3>{1.f, 1.f, 0.f}};
    ColoredVertex v01{
        .position = FixedArray<float, 3>{pos2d(0) - size, pos2d(1) + size, 0.f},
        .color = FixedArray<float, 3>{1.f, 0.f, 1.f}};
    ColoredVertex v10{
        .position = FixedArray<float, 3>{pos2d(0) + size, pos2d(1) - size, 0.f},
        .color = FixedArray<float, 3>{0.f, 1.f, 1.f}};
    ColoredVertex v11{
        .position = FixedArray<float, 3>{pos2d(0) + size, pos2d(1) + size, 0.f},
        .color = FixedArray<float, 3>{1.f, 1.f, 1.f}};

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});
}

void Mlib::draw_nodes(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, std::list<std::string>>& ways)
{
    for (const auto& way : ways) {
        for (const auto& nd : way.second) {
            if (nodes.find(nd) == nodes.end()) {
                throw std::runtime_error("Way " + way.first + " could not find node with ID " + nd);
            }
            FixedArray<float, 2> pos2d = nodes.at(nd).position;
            draw_node(triangles, pos2d);
        }
    }
}

// void Mlib::draw_test_lines(
//     TriangleList& tl,
//     float width)
// {
//     Rectangle rect;
//     if (!Rectangle::from_line(
//         rect,
//         FixedArray<float, 2>{0, 0},
//         FixedArray<float, 2>{0, 0},
//         FixedArray<float, 2>{0.1, 0},
//         FixedArray<float, 2>{0.2, 0},
//         FixedArray<float, 2>{0.3, 0},
//         FixedArray<float, 2>{0.3, 0},
//         width,
//         width,
//         width,
//         width,
//         width))
//     {
//         throw std::runtime_error("from_line failed");
//     }
//     rect.draw_z0(tl);
//     if (!Rectangle::from_line(
//         rect,
//         FixedArray<float, 2>{-0.1, 0},
//         FixedArray<float, 2>{-0.1, 0},
//         FixedArray<float, 2>{-0.2, 0},
//         FixedArray<float, 2>{-0.3, 0},
//         FixedArray<float, 2>{-0.4, 0},
//         FixedArray<float, 2>{-0.4, 0},
//         width,
//         width,
//         width,
//         width,
//         width));
//     {
//         throw std::runtime_error("from_line failed");
//     }
//     rect.draw_z0(tl);
// }

float Mlib::parse_meters(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value)
{
    auto it = tags.find(key);
    if (it == tags.end()) {
        return default_value;
    }
    static const DECLARE_REGEX(re, "^([\\d.-]+) *(m|')?");
    Mlib::re::smatch match;
    if (Mlib::re::regex_match(it->second, match, re)) {
        float res = safe_stof(match[1].str());
        if (match[2].str() == "'") {
            res *= 0.3048f;
        }
        return res;
    } else {
        throw std::runtime_error("Could not parse height value: " + it->second);
    }
}

FixedArray<float, 3> Mlib::parse_color(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    const FixedArray<float, 3>& default_value)
{
    auto rgb_it = tags.find(key);
    if (rgb_it != tags.end()) {
        auto l = string_to_vector(rgb_it->second, safe_stof);
        if (l.size() != 3) {
            throw std::runtime_error("\"color\" tag does not have 3 values");
        }
        return FixedArray<float, 3>{l[0], l[1], l[2]};
    } else {
        return default_value;
    }
}

float Mlib::parse_float(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    float default_value)
{
    auto it = tags.find(key);
    if (it != tags.end()) {
        return safe_stof(it->second);
    } else {
        return default_value;
    }
}

bool Mlib::parse_bool(
    const std::map<std::string, std::string>& tags,
    const std::string& key,
    bool default_value)
{
    auto it = tags.find(key);
    if (it != tags.end()) {
        return safe_stob(it->second);
    } else {
        return default_value;
    }
}

void Mlib::draw_roofs(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const FixedArray<float, 3>& color,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float width,
    float scale,
    float z0,
    float z1)
{
    for (const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            throw std::runtime_error("Building " + bu.id + ": outline not closed");
        }
        tls.push_back(std::make_shared<TriangleList>("roofs", material));
        auto way1 = bu.way.nd;
        way1.erase(way1.begin());
        float zz0 = z0;
        float zz1 = z1;
        if (bu.area < 0) {
            std::swap(zz0, zz1);
        }
        auto a = way1.begin();
        for (size_t i = 0; i < way1.size(); ++i) {
            auto b = a;
            ++b;
            if (b == way1.end()) {
                b = way1.begin();
            }
            auto c = b;
            ++c;
            if (c == way1.end()) {
                c = way1.begin();
            }
            auto d = c;
            ++d;
            if (d == way1.end()) {
                d = way1.begin();
            }
            Rectangle rect;
            if (!Rectangle::from_line(
                    rect,
                    nodes.at(*a).position,
                    nodes.at(*a).position,
                    nodes.at(*b).position,
                    nodes.at(*c).position,
                    nodes.at(*d).position,
                    nodes.at(*d).position,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width,
                    scale * width))
            {
                std::cerr << "Error triangulating roof " + bu.id << std::endl;
            } else {
                rect.draw_z(*tls.back(), zz0 * scale, zz1 * scale, color);
            }
            // draw_node(triangles, nodes.at(*a));
            ++a;
            if (a == way1.end()) {
                a = way1.begin();
            }
        }
    }
}

void Mlib::draw_buildings_ceiling_or_ground(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    DrawBuildingPartType tpe)
{
    for (const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            throw std::runtime_error("Building " + bu.id + ": outline not closed");
        }
        if ((tpe == DrawBuildingPartType::GROUND) &&
            bu.way.tags.contains("layer") &&
            (safe_stoi(bu.way.tags.at("layer")) != 0))
        {
            continue;
        }
        auto sw = smooth_way(nodes, bu.way.nd, scale, max_width);
        if (sw.empty()) {
            throw std::runtime_error("Smoothed outline is empty");
        }
        std::vector<FixedArray<float, 2>> outline;
        outline.reserve(sw.size() - 1);
        auto it = sw.begin();
        ++it;
        for (; it != sw.end(); ++it) {
            outline.push_back(*it);
        }
        outline = removed_duplicates(outline);
        if (bu.area == 0.f) {
            throw std::runtime_error("Building area not computed");
        }
        if (bu.area < 0.f) {
            std::reverse(outline.begin(), outline.end());
        }
        tls.push_back(std::make_shared<TriangleList>("ceilings", material));
        TerrainTypeTriangleList tl_terrain;
        tl_terrain.insert(TerrainType::UNDEFINED, tls.back());
        BoundingInfo bounding_info{outline, {}, 0.1f};
        try {
            triangulate_terrain_or_ceilings(
                tl_terrain,                                                 // tl_terrain
                bounding_info,                                              // bounding_info
                {},                                                         // steiner_points
                outline,                                                    // bounding_contour
                {},                                                         // hole_triangles
                {},                                                         // region_contours
                scale,                                                      // scale
                uv_scale,                                                   // uv_scale
                tpe == DrawBuildingPartType::CEILING ? bu.building_top : 0, // z
                parse_color(bu.way.tags, "color", building_color),          // color
                "",                                                         // contour_filename
                "",                                                         // triangle_filename
                TerrainType::UNDEFINED);
        } catch (const std::runtime_error& e) {
            throw std::runtime_error("Could not triangulate building " + bu.id + ": " + e.what());
        }
    }
}

//class PolygonDrawer {
//public:
//    void draw_line(const p2t::Point& from, const p2t::Point& to, size_t nsteps) {
//        for (size_t i = 0; i < nsteps; ++i) {
//            double alpha = double(i) / nsteps;
//            point_list_.push_back((1 - alpha) * from + alpha * to);
//            points_.push_back(&point_list_.back());
//        }
//    }
//    std::vector<p2t::Point*> points_;
//private:
//    std::list<p2t::Point> point_list_;
//};

void Mlib::raise_streets(
    const std::list<std::shared_ptr<TriangleList>>& tls_street_wo_curb,
    const std::list<std::shared_ptr<TriangleList>>& tls_ground,
    float scale,
    float amount)
{
    std::set<OrderableFixedArray<float, 3>> raised_nodes;
    for (auto& l : tls_street_wo_curb) {
        for (const auto& n : l->triangles_) {
            raised_nodes.insert(OrderableFixedArray{n(0).position});
            raised_nodes.insert(OrderableFixedArray{n(1).position});
            raised_nodes.insert(OrderableFixedArray{n(2).position});
        }
    }
    for (auto& l : tls_ground) {
        for (auto& n : l->triangles_) {
            for (auto& v : n.flat_iterable()) {
                if (raised_nodes.find(OrderableFixedArray{v.position}) != raised_nodes.end()) {
                    v.position(2) += scale * amount;
                }
            }
        }
    }
}

void Mlib::add_grass_on_steiner_points(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const std::list<SteinerPointInfo>& steiner_points,
    float scale,
    float dmin,
    float dmax)
{
    NormalRandomNumberGenerator<float> scale_rng{0, 1.f, 0.2f};
    for (const auto& p : steiner_points) {
        if ((p.type == SteinerPointType::STREET_NEIGHBOR) &&
            !std::isnan(p.distance_to_road) &&
            ((p.distance_to_road > dmin * scale) &&
             (p.distance_to_road < dmax * scale) &&
             (p.distance_to_air_road > dmin * scale)))
        {
            const ParsedResourceName* prn = rnc.try_once();
            if (prn != nullptr) {
                add_parsed_resource_name(p.position, *prn, scale_rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            }
        }
    }
}

void Mlib::add_grass_inside_triangles(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    ResourceNameCycle& rnc,
    const TriangleList& triangles,
    float scale,
    float distance)
{
    if (distance == INFINITY) {
        return;
    }
    TriangleSampler2<float> ts{ 1 };
    NormalRandomNumberGenerator<float> rng{ 0, 1.f, 0.2f };
    for (auto& t : triangles.triangles_) {
        ts.sample_triangle_interior<3>(
            t(0).position,
            t(1).position,
            t(2).position,
            distance * scale,
            [&](float a, float b, float c)
            {
                FixedArray<float, 3> p = t(0).position * a + t(1).position * b + t(2).position * c;
                add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            });
    }
}

void Mlib::add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float tree_distance,
    float tree_inwards_distance,
    float scale)
{
    NormalRandomNumberGenerator<float> rng{0, 1.f, 0.2f};
    NormalRandomNumberGenerator<float> rng2{0, 0.f, 1.2f};
    // size_t rid = 0;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if ((tags.find("landuse") != tags.end() && tags.at("landuse") == "forest") ||
            (tags.find("natural") != tags.end() && tags.at("natural") == "wood"))
        {
            float area = compute_area_clockwise(w.second.nd, nodes, scale);
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s == w.second.nd.end()) {
                    continue;
                }
                FixedArray<float, 2> p0 = nodes.at(*it).position;
                FixedArray<float, 2> p1 = nodes.at(*s).position;
                float len = std::sqrt(sum(squared(p0 - p1)));
                FixedArray<float, 2> n{p0(1) - p1(1), p1(0) - p0(0)};
                n /= len;
                for (float a = 0.1f; a < 0.91f; a += tree_distance * scale / len) {
                    float aa = a + rng2() * tree_distance * scale / len;
                    if (aa < 0 || aa > 0.91f) {
                        continue;
                    }
                    FixedArray<float, 2> p = (aa * p0 + (1 - aa) * p1) - tree_inwards_distance * scale * n * sign(area);
                    if (std::isnan(min_dist_to_road) || !ground_bvh.has_neighbor(p, min_dist_to_road * scale)) {
                        add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
                        // object_resource_descriptors.push_back({
                        //     position: FixedArray<float, 3>{p(0), p(1), 0},
                        //     name: rnc(),
                        //     scale: rng()});
                        // if ((rid++) % 4 == 0) {
                        steiner_points.push_back({
                            .position = {p(0), p(1), 0.f},
                            .type = SteinerPointType::FOREST_OUTLINE,
                            .distance_to_road = NAN});
                        // }
                    }
                }
            }
        }
    }
}

void Mlib::add_beacons_to_raceways(
    std::list<ObjectResourceDescriptor>& street_light_positions,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float raceway_beacon_distance,
    float scale)
{
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("raceway") != tags.end() && tags.at("raceway") == "yes")
        {
            auto sw = smooth_way(nodes, w.second.nd, scale, raceway_beacon_distance);
            for (const auto p : sw) {
                street_light_positions.push_back({FixedArray<float, 3>{p(0), p(1), 0.f}, "raceway_beacon", 1.f});
            }
        }
    }
}

// void Mlib::add_grass_outlines(
//     std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
//     std::list<FixedArray<float, 2>>& steiner_points,
//     const std::map<std::string, Node>& nodes,
//     const std::map<std::string, Way>& ways,
//     bool continuous,
//     float tree_distance,
//     float tree_inwards_distance,
//     float scale)
// {
//     NormalRandomNumberGenerator<float> rng{0, 4.9, 0.2};
//     NormalRandomNumberGenerator<float> rng2{0, 0, 1.2};
//     GammaRandomNumberGenerator<float> rng3{0, 3, 2};
//     NextGrassResourceName ntrn{continuous};
//     for (float f : linspace<float>(tree_inwards_distance, tree_inwards_distance + 5.f, 3).flat_iterable()) {
//         for (const auto& w : ways) {
//             const auto& tags = w.second.tags;
//             if ((tags.find("landuse") != tags.end() && (tags.at("landuse") == "farmland" || tags.at("landuse") == "meadow")))
//             {
//                 float area = compute_area_clockwise(w.second.nd, nodes, scale);
//                 for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
//                     auto s = it;
//                     ++s;
//                     if (s == w.second.nd.end()) {
//                         continue;
//                     }
//                     FixedArray<float, 2> p0 = nodes.at(*it).position;
//                     FixedArray<float, 2> p1 = nodes.at(*s).position;
//                     float len = std::sqrt(sum(squared(p0 - p1)));
//                     FixedArray<float, 2> n{p0(1) - p1(1), p1(0) - p0(0)};
//                     n /= len;
//                     for (float a = 0.1; a < 0.91; a += tree_distance * scale / len) {
//                         float aa = a + rng2() * tree_distance * scale / len;
//                         if (aa < 0 || aa > 0.91) {
//                             continue;
//                         }
//                         FixedArray<float, 2> p = (aa * p0 + (1 - aa) * p1) + (f + rng3()) * scale * n * sign(area);
//                         resource_instance_positions.push_back({FixedArray<float, 3>{p(0), p(1), 0}, ntrn(), rng()});
//                     }
//                 }
//             }
//         }
//     }
// }

void Mlib::add_trees_to_tree_nodes(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    float min_dist_to_road,
    const StreetBvh& ground_bvh,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    NormalRandomNumberGenerator<float> rng{0, 1.f, 0.2f};
    for (const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (tags.find("natural") != tags.end() && tags.at("natural") == "tree") {
            const auto& p = n.second.position;
            if (std::isnan(min_dist_to_road) || !ground_bvh.has_neighbor(p, min_dist_to_road * scale)) {
                add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
                steiner_points.push_back({
                    .position = {p(0), p(1), 0.f},
                    .type = SteinerPointType::TREE_NODE,
                    .distance_to_road = NAN});
            }
        }
    }
}

void Mlib::add_binary_vegetation_old(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const std::string& grass_texture,
    const std::string& tree_texture,
    const std::string& tree_texture_2,
    const TriangleList& ground_triangles,
    float scale)
{
    size_t tid = 0;
    for (auto& t : ground_triangles.triangles_) {
        ++tid;
        tls.push_back(std::make_shared<TriangleList>("binary_vegetation_old", material));
        float veg_size;
        switch (tid % 10) {
            case 0:
                tls.back()->material_.textures = { {.texture_descriptor = {.color = grass_texture}} };
                veg_size = 1;
                break;
            case 2:
                tls.back()->material_.textures = { {.texture_descriptor = {.color = tree_texture}} };
                veg_size = 5;
                break;
            case 4:
                tls.back()->material_.textures = { {.texture_descriptor = {.color = tree_texture_2 }} };
                veg_size = 5;
                break;
            default:
                continue;
                // throw std::runtime_error("Internal error");
        }
        auto center = (t(0).position + t(1).position + t(2).position) / 3.f;
        tls.back()->draw_rectangle_wo_normals(
            center + FixedArray<float, 3>{-scale * veg_size, 0.f, 0.f},
            center + FixedArray<float, 3>{scale * veg_size, 0.f, 0.f},
            center + FixedArray<float, 3>{scale * veg_size, 0.f, 2 * scale * veg_size},
            center + FixedArray<float, 3>{-scale * veg_size, 0.f, 2 * scale * veg_size},
            {1.f, 1.f, 1.f },
            {1.f, 1.f, 1.f },
            {1.f, 1.f, 1.f });
        tls.back()->draw_rectangle_wo_normals(
            center + FixedArray<float, 3>{0.f, -scale * veg_size, 0.f},
            center + FixedArray<float, 3>{0.f, scale * veg_size, 0.f},
            center + FixedArray<float, 3>{0.f, scale * veg_size, 2 * scale * veg_size},
            center + FixedArray<float, 3>{0.f, -scale * veg_size, 2 * scale * veg_size},
            {1.f, 1.f, 1.f },
            {1.f, 1.f, 1.f },
            {1.f, 1.f, 1.f });
    }
}

/*enum class TriangleMaterial {
    ROAD,
    TERRAIN
};

template <class TTag>
class TriangleTag {
    using O = OrderableFixedArray<float, 3>;
public:
    TTag get(const FixedArray<ColoredVertex, 3>& triangle) const {
        OrderableFixedArray<O, 3> key;
        key(0) = O{triangle(0).position};
        key(1) = O{triangle(1).position};
        key(2) = O{triangle(2).position};
        return tags_.at(key);
    }
    void set(const FixedArray<ColoredVertex, 3>& triangle, const TTag& tag) {
        OrderableFixedArray<OrderableFixedArray<float, 3>, 3> key{
            triangle(0).position,
            triangle(1).position,
            triangle(2).position};
        auto it = tags_.find(key);
        if (it != tags_.end()) {
            throw std::runtime_error("Tag already set");
            tags_.insert(key, tag);
        }
    }
private:
    std::map<OrderableFixedArray<OrderableFixedArray<float, 3>, 3>, TriangleMaterial> tags_;
};*/

void Mlib::colorize_height_map(std::list<FixedArray<ColoredVertex, 3>>& triangles)
{
    StaticFaceLightning sfl{true}; // true == swap_yz
    for (auto& t : triangles) {
        t(0).color = sfl.get_color(t(0).color, t(0).normal);
        t(1).color = sfl.get_color(t(1).color, t(1).normal);
        t(2).color = sfl.get_color(t(2).color, t(2).normal);
    }
}

void Mlib::compute_building_area(
    std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    for (auto& b : buildings) {
        b.area = compute_area_clockwise(b.way.nd, nodes, scale);
        // if (b.area < 0.f) {
        //     std::cerr << "Negative building area: ID " << b.id << " area " << b.area << std::endl;
        // }
    }
}

void Mlib::draw_building_walls(
    std::list<std::shared_ptr<TriangleList>>& tls,
    std::list<SteinerPointInfo>* steiner_points,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width,
    const std::vector<std::string>& facade_textures)
{
    if (facade_textures.empty()) {
        throw std::runtime_error("Facade textures empty");
    }
    size_t bid = 0;
    for (const auto& bu : buildings) {
        ++bid;
        tls.push_back(std::make_shared<TriangleList>("building_walls", material));
        tls.back()->material_.textures = { {.texture_descriptor = {.color = facade_textures.at(bid % facade_textures.size())}} };
        tls.back()->material_.compute_color_mode();
        FixedArray<float, 3> color = parse_color(bu.way.tags, "color", building_color);
        auto sw = smooth_way(nodes, bu.way.nd, scale, max_width);
        for (auto it = sw.begin(); it != sw.end(); ++it) {
            auto s = it;
            ++s;
            if (s != sw.end()) {
                const auto& p0 = bu.area < 0 ? *s : *it;
                const auto& p1 = bu.area < 0 ? *it : *s;
                float width = std::sqrt(sum(squared(p0 - p1)));
                float height = (bu.building_top - bu.building_bottom) * scale;
                if (steiner_points != nullptr) {
                    steiner_points->push_back({
                        .position = {p0(0), p0(1), 0.f},
                        .type = SteinerPointType::WALL,
                        .distance_to_road = NAN});
                }
                // some buildings are clock-wise, others counter-clock-wise
                tls.back()->draw_rectangle_wo_normals(
                    {p1(0), p1(1), bu.building_bottom * scale},
                    {p0(0), p0(1), bu.building_bottom * scale},
                    {p0(0), p0(1), bu.building_top * scale},
                    {p1(0), p1(1), bu.building_top * scale},
                    color,
                    color,
                    color,
                    color,
                    {0.f, 0.f},
                    {width / scale * uv_scale, 0.f},
                    {width / scale * uv_scale, height / scale * uv_scale},
                    {0.f, height / scale * uv_scale});
            }
        }
    }
}

template <class TContainer, class TGetOrderableFixedArray>
void to_orderable_fixed_array(
    TContainer& result,
    const TContainer& nodes,
    bool verbose,
    const TGetOrderableFixedArray& get_orderable_fixed_array)
{
    typedef decltype(get_orderable_fixed_array(nodes.front())) Key;
    std::set<Key> pts;
    for (const auto& p : nodes) {
        auto o = get_orderable_fixed_array(p);
        if (pts.find(o) != pts.end()) {
            if (verbose) {
                std::cerr << "Removing duplicate point " << o << std::endl;
            }
        } else {
            pts.insert(o);
            result.push_back(p);
        }
    }
}

std::vector<FixedArray<float, 2>> Mlib::removed_duplicates(
    const std::vector<FixedArray<float, 2>>& nodes,
    bool verbose)
{
    std::vector<FixedArray<float, 2>> result;
    result.reserve(nodes.size());
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const FixedArray<float, 2>& p){return OrderableFixedArray<float, 2>{p};});
    return result;
}

std::list<FixedArray<float, 2>> Mlib::removed_duplicates(
    const std::list<FixedArray<float, 2>>& nodes,
    bool verbose)
{
    std::list<FixedArray<float, 2>> result;
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const FixedArray<float, 2>& p){return OrderableFixedArray<float, 2>{p};});
    return result;
}

std::list<SteinerPointInfo> Mlib::removed_duplicates(
    const std::list<SteinerPointInfo>& nodes,
    bool verbose)
{
    std::list<SteinerPointInfo> result;
    to_orderable_fixed_array(
        result,
        nodes,
        verbose,
        [](const SteinerPointInfo& p){return OrderableFixedArray<float, 3>{p.position};});
    return result;
}

ResourceNameCycle::ResourceNameCycle(
    const SceneNodeResources& resources,
    const std::vector<std::string>& names)
: index_{1, 0, names.size() - 1},
  probability_{1234321}
{
    static const DECLARE_REGEX(re, "^(.*?)\\(p:([\\d+.e-]+)\\)(?:\\(hitbox:(\\w+)\\))?$");
    names_.reserve(names.size());
    for (const std::string& name : names) {
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(name, match, re)) {
            names_.push_back(ParsedResourceName{
                .name = match[1].str(),
                .probability = safe_stof(match[2].str()),
                .aggregate_mode = resources.aggregate_mode(match[1].str()),
                .hitbox = match[3].str()});
            if (names_.back().probability < 1e-7) {
                throw std::runtime_error("ResourceNameCycle: threshold too small");
            }
            if (names_.back().probability > 1) {
                throw std::runtime_error("ResourceNameCycle: threshold too large");
            }
        } else {
            names_.push_back(ParsedResourceName{
                .name = name,
                .probability = 1,
                .aggregate_mode = resources.aggregate_mode(name)});
        }
    }
}

ResourceNameCycle::~ResourceNameCycle()
{}

const ParsedResourceName* ResourceNameCycle::try_once() {
    const ParsedResourceName& prn = names_[index_()];
    if (prn.probability != 1) {
        if (probability_() > prn.probability) {
            return nullptr;
        }
    }
    return &prn;
}

const ParsedResourceName& ResourceNameCycle::operator() () {
    if (names_.empty()) {
        throw std::runtime_error("ResourceNameCycle called with empty names");
    }
    const ParsedResourceName* res = nullptr;
    while(res == nullptr) {
        res = try_once();
    }
    return *res;
}

bool ResourceNameCycle::empty() const {
    return names_.empty();
}

void ResourceNameCycle::seed(unsigned int seed) {
    index_.seed(seed);
    probability_.seed(seed);
}

void Mlib::check_curb_validity(float curb_alpha, float curb2_alpha) {
    if (curb_alpha > curb2_alpha) {
        throw std::runtime_error("curb_alpha > curb2_alpha");
    }
    if (curb_alpha <= 0) {
        throw std::runtime_error("curb_alpha <= 0");
    }
    if (curb2_alpha > 1) {
        throw std::runtime_error("curb2_alpha > 1");
    }
}
