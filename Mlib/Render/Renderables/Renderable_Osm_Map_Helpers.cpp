#include "Renderable_Osm_Map_Helpers.hpp"
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Intersection/Point_Triangle_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Contour.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Geometry/Static_Face_Lightning.hpp>
#include <Mlib/Geometry/Triangle_Is_Right_Handed.hpp>
#include <Mlib/Geometry/Triangle_Normal.hpp>
#include <Mlib/Images/Bilinear_Interpolation.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Render/Renderables/Renderable_Osm_Map_Rectangle.hpp>
#include <Mlib/Render/Renderables/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Stats/Linspace.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/String.hpp>
#include <fstream>
#include <poly2tri/poly2tri.h>
#include <regex>

using namespace Mlib;

static std::list<FixedArray<float, 2>> smooth_way(
    const std::map<std::string, Node>& nodes,
    const std::list<std::string>& nd,
    float scale,
    float max_width)
{
    std::list<FixedArray<float, 2>> result;
    for(auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            auto p0 = nodes.at(*it).position;
            auto p1 = nodes.at(*s).position;
            float width = std::sqrt(sum(squared(p0 - p1)));
            auto refined = linspace(0.f, 1.f, std::max(2, int(width / scale / max_width))).flat_iterable();
            for(auto a = refined.begin(); a != refined.end(); ++a) {
                auto b = a;
                ++b;
                if (b != refined.end() || &*s == &nd.back()) {
                    auto pp0 = (1 - *a) * p0 + (*a) * p1;
                    result.push_back(pp0);
                }
            }
        }
    }
    return result;
}

static float compute_area(
    const std::list<std::string>& nd,
    const std::map<std::string, Node>& nodes,
    float scale)
{
    // Source: https://stackoverflow.com/questions/1165647/how-to-determine-if-a-list-of-polygon-points-are-in-clockwise-order
    float area2 = 0;
    for(auto it = nd.begin(); it != nd.end(); ++it) {
        auto s = it;
        ++s;
        if (s != nd.end()) {
            const auto& a = nodes.at(*s).position;
            const auto& b = nodes.at(*it).position;
            area2 += (b(0) - a(0)) * (b(1) + a(1));
        }
    }
    return area2 / 2 / squared(scale);
}

// std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> Mlib::gen_steiner_point_map(std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, SteinerPointInfo*> steiner_point_map;
//     for(auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             throw std::runtime_error("Could not generate steiner point map");
//         }
//     }
//     return steiner_point_map;
// }
// 
// std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> Mlib::gen_const_steiner_point_map(const std::list<SteinerPointInfo>& steiner_points) {
//     std::map<OrderableFixedArray<float, 3>, const SteinerPointInfo*> steiner_point_map;
//     for(auto& p : steiner_points) {
//         if (!steiner_point_map.insert({OrderableFixedArray{p.position}, &p}).second) {
//             throw std::runtime_error("Could not generate const steiner point map");
//         }
//     }
//     return steiner_point_map;
// }

static void add_parsed_resource_name(
    const FixedArray<float, 3>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes)
{
    if (prn.aggregate_mode & (AggregateMode::INSTANCES_ONCE | AggregateMode::INSTANCES_SORTED_CONTINUOUSLY)) {
        resource_instance_positions[prn.name].push_back({p, scale});
    } else {
        object_resource_descriptors.push_back({p, prn.name, scale});
    }
    if (!prn.hitbox.empty()) {
        hitboxes[prn.hitbox].push_back(p);
    }
}

static void add_parsed_resource_name(
    const FixedArray<float, 2>& p,
    const ParsedResourceName& prn,
    float scale,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes)
{
    add_parsed_resource_name(
        FixedArray<float, 3>{p(0), p(1), 0},
        prn,
        scale,
        resource_instance_positions,
        object_resource_descriptors,
        hitboxes);
}

void Mlib::draw_node(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const FixedArray<float, 2>& pos2d,
    float size)
{
    ColoredVertex v00{
        position: FixedArray<float, 3>{pos2d(0) - size, pos2d(1) - size, 0},
        color: FixedArray<float, 3>{1, 1, 0}};
    ColoredVertex v01{
        position: FixedArray<float, 3>{pos2d(0) - size, pos2d(1) + size, 0},
        color: FixedArray<float, 3>{1, 0, 1}};
    ColoredVertex v10{
        position: FixedArray<float, 3>{pos2d(0) + size, pos2d(1) - size, 0},
        color: FixedArray<float, 3>{0, 1, 1}};
    ColoredVertex v11{
        position: FixedArray<float, 3>{pos2d(0) + size, pos2d(1) + size, 0},
        color: FixedArray<float, 3>{1, 1, 1}};

    triangles.push_back(FixedArray<ColoredVertex, 3>{v00, v11, v01});
    triangles.push_back(FixedArray<ColoredVertex, 3>{v11, v00, v10});
}

void Mlib::draw_nodes(
    std::vector<FixedArray<ColoredVertex, 3>>& triangles,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, std::list<std::string>>& ways)
{
    for(const auto& way : ways) {
        for(const auto& nd : way.second) {
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

float Mlib::parse_meters(const std::map<std::string, std::string>& tags, const std::string& key, float default_value) {
    auto it = tags.find(key);
    if (it == tags.end()) {
        return default_value;
    }
    std::regex re{"^([\\d.-]+) *(m|')?"};
    std::smatch match;
    if (std::regex_match(it->second, match, re)) {
        float res = safe_stof(match[1].str());
        if (match[2].str() == "'") {
            res *= 0.3048;
        }
        return res;
    } else {
        throw std::runtime_error("Could not parse height value: " + it->second);
    }
}

std::list<Building> Mlib::get_buildings_or_wall_barriers(
    BuildingType building_type,
    const std::map<std::string, Way>& ways,
    float building_bottom,
    float default_building_top)
{
    std::list<Building> result;
    for(const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("level") != tags.end()) {
            continue;
        }
        if (tags.find("building") != tags.end()) {
            if (building_type != BuildingType::BUILDING) {
                continue;
            }
        } else if ((tags.find("barrier") != tags.end()) && (included_barriers.contains(tags.at("barrier")))) {
            if (building_type != BuildingType::WALL_BARRIER) {
                continue;
            }
        } else if ((tags.find("spawn_line") != tags.end()) && (tags.at("spawn_line") == "yes")) {
            if (building_type != BuildingType::SPAWN_LINE) {
                continue;
            }
        } else if ((tags.find("way_points") != tags.end()) && (tags.at("way_points") == "yes")) {
            if (building_type != BuildingType::WAYPOINTS) {
                continue;
            }
        } else {
            continue;
        }
        float building_top = default_building_top;
        building_top = parse_meters(tags, "height", building_top);
        building_top = parse_meters(tags, "building:height", building_top);
        result.push_back(Building{
            id: w.first,
            way: w.second,
            building_top: building_top,
            building_bottom: building_bottom});
    }
    return result;
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
    for(const auto& bu : buildings) {
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
        if (bu.area > 0) {
            std::swap(zz0, zz1);
        }
        auto a = way1.begin();
        for(size_t i = 0; i < way1.size(); ++i) {
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

void Mlib::draw_ceilings(
    std::list<std::shared_ptr<TriangleList>>& tls,
    const Material& material,
    const std::list<Building>& buildings,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float max_width)
{
    for(const auto& bu : buildings) {
        if (bu.way.nd.empty()) {
            std::cerr << "Building " + bu.id + ": outline is empty" << std::endl;
            continue;
        }
        if (bu.way.nd.front() != bu.way.nd.back()) {
            throw std::runtime_error("Building " + bu.id + ": outline not closed");
        }
        auto sw = smooth_way(nodes, bu.way.nd, scale, max_width);
        std::vector<FixedArray<float, 2>> outline;
        outline.reserve(sw.size() - 1);
        auto it = sw.begin();
        ++it;
        for(; it != sw.end(); ++it) {
            outline.push_back(*it);
        }
        outline = removed_duplicates(outline);
        //std::reverse(outline.begin(), outline.end());
        tls.push_back(std::make_shared<TriangleList>("ceilings", material));
        BoundingInfo bounding_info{outline, {}, 0.1};
        triangulate_terrain_or_ceilings(
            *tls.back(),
            bounding_info,
            {},
            outline,
            {},
            {},
            scale,
            uv_scale,
            bu.building_top);
    }
}

void Mlib::get_neighbors(
    const std::string& center,
    const std::map<std::string, NeighborWay>& neighbors,
    const std::map<float, AngleWay>& angles,
    const std::string** l,
    const std::string** r)
{
    float angle = neighbors.at(center).angle;
    auto it = angles.find(angle);
    if (it == angles.end()) {
        throw std::runtime_error("Could not find angle");
    }
    if (it == angles.begin()) {
        *l = &angles.rbegin()->second.id;
    } else {
        auto itL = it;
        *l = &(--itL)->second.id;
    }
    auto itR = it;
    ++itR;
    if (itR == angles.end()) {
        *r = &angles.begin()->second.id;
    } else {
        *r = &itR->second.id;
    }
}

//class PolygonDrawer {
//public:
//    void draw_line(const p2t::Point& from, const p2t::Point& to, size_t nsteps) {
//        for(size_t i = 0; i < nsteps; ++i) {
//            double alpha = double(i) / nsteps;
//            point_list_.push_back((1 - alpha) * from + alpha * to);
//            points_.push_back(&point_list_.back());
//        }
//    }
//    std::vector<p2t::Point*> points_;
//private:
//    std::list<p2t::Point> point_list_;
//};

void Mlib::get_map_outer_contour(
    std::vector<FixedArray<float, 2>>& contour,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    if (!contour.empty()) {
        throw std::runtime_error("Initial map contour not empty");
    }
    for(const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("name") != tags.end() && tags.at("name") == "map-outer-contour") {
            if (!contour.empty()) {
                throw std::runtime_error("Found multiple map contours");
            }
            contour.reserve(w.second.nd.size());
            if (w.second.nd.empty()) {
                throw std::runtime_error("Map outer contour is empty");
            }
            for(auto it = w.second.nd.begin(); ; ++it) {
                auto s = it;
                ++s;
                if (s == w.second.nd.end()) {
                    if (*it != *w.second.nd.begin()) {
                        throw std::runtime_error("Map outer contour not closed");
                    }
                    break;
                }
                contour.push_back(nodes.at(*it).position);
           }
        }
    }
}

struct AngleCurb {
    float angle;
    size_t curb;
    auto operator <=> (const AngleCurb& other) const = default;
};

void Mlib::draw_streets(
    TriangleList& tl_street_crossing,
    TriangleList& tl_path_crossing,
    TriangleList& tl_street,
    TriangleList& tl_path,
    TriangleList& tl_curb_street,
    TriangleList& tl_curb_path,
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float scale,
    float uv_scale,
    float default_street_width,
    bool only_raceways,
    const std::string& name_pattern,
    const std::set<std::string>& excluded_highways,
    const std::set<std::string>& path_tags,
    float curb_alpha,
    ResourceNameCycle& street_lights,
    bool with_height_bindings)
{
    std::regex name_re{name_pattern};
    std::map<std::string, std::map<float, AngleWay>> node_angles;
    std::map<std::string, std::map<std::string, NeighborWay>> node_neighbors;
    std::map<std::string, std::map<AngleCurb, FixedArray<float, 2>>> node_hole_contours;
    for(const auto& n : nodes) {
        node_angles.insert(std::make_pair(n.first, std::map<float, AngleWay>()));
        node_neighbors.insert(std::make_pair(n.first, std::map<std::string, NeighborWay>()));
        node_hole_contours.insert(std::make_pair(n.first, std::map<AngleCurb, FixedArray<float, 2>>()));
    }
    std::map<std::string, NodeWayInfo> node_way_info;
    std::set<std::string> node_no_way_length;
    for(const auto& w : ways) {
        FixedArray<float, 3> color;
        auto rgb_it = w.second.tags.find("color");
        if (rgb_it != w.second.tags.end()) {
            auto l = string_to_vector(rgb_it->second);
            if (l.size() != 3) {
                throw std::runtime_error("\"color\" tag does not have 3 values");
            }
            color(0) = safe_stof(l[0]);
            color(1) = safe_stof(l[1]);
            color(2) = safe_stof(l[2]);
        } else {
            color = way_color;
        }
        const auto& tags = w.second.tags;
        {
            bool exclude_by_tag = false;
            for(const auto& x : excluded_highway_tags) {
                if (tags.find(x) != tags.end()) {
                    exclude_by_tag = true;
                    break;
                }
            }
            if (exclude_by_tag) {
                continue;
            }
        }
        if (tags.find("highway") != tags.end() && (!excluded_highways.contains(tags.at("highway")))) {
            if (only_raceways && (tags.at("highway") != "raceway")) {
                continue;
            }
            if (!name_pattern.empty() && ((tags.find("name") == tags.end()) || !std::regex_match(tags.at("name"), name_re))) {
                continue;
            }
            if (tags.find("area") != tags.end() && tags.at("area") == "yes") {
                continue;
            }
            float width = scale * parse_meters(tags, "width", default_street_width);
            RoadType road_type = path_tags.contains(tags.at("highway")) ? RoadType::PATH : RoadType::STREET;
            float way_length = 0;
            for(auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                if (node_no_way_length.find(*it) == node_no_way_length.end()) {
                    if (node_way_info.find(*it) != node_way_info.end()) {
                        node_no_way_length.insert(*it);
                        node_way_info.erase(*it);
                    } else {
                        node_way_info.insert(std::make_pair(*it, NodeWayInfo{way_length: way_length, color: color}));
                    }
                }
                auto s = it;
                ++s;
                if (s != w.second.nd.end()) {
                    FixedArray<float, 2> dir = nodes.at(*it).position - nodes.at(*s).position;
                    float angle0 = std::atan2(dir(1), dir(0));
                    float angle1 = std::atan2(-dir(1), -dir(0));
                    node_angles.at(*it).insert(std::make_pair(angle0, AngleWay{*s, width, road_type}));
                    node_angles.at(*s).insert(std::make_pair(angle1, AngleWay{*it, width, road_type}));
                    node_neighbors.at(*it).insert(std::make_pair(*s, NeighborWay{angle0, width}));
                    node_neighbors.at(*s).insert(std::make_pair(*it, NeighborWay{angle1, width}));
                    way_length += std::sqrt(sum(squared(nodes.at(*s).position - nodes.at(*it).position)));
                }
            }
        }
    }

    Bvh<float, bool, 2> bvh{{0.1, 0.1}, 10};

    // Compute rectangles and holes for each pair of connected nodes.
    // To avoid duplicates, the computations are done at each lexicographically
    // smaller node.
    for(const auto& na : node_angles) {
        for(auto it = na.second.begin(); it != na.second.end(); ++it) {
            if (it->second.id < na.first) {
                // node index: na.first
                // neighbor index: it->second
                // angle of neighbor: it->first
                // angle at neighbor: node_neighbors.at(it->second).at(na.first)
                const std::string* aL;
                const std::string* aR;
                get_neighbors(it->second.id, node_neighbors.at(na.first), na.second, &aL, &aR);
                const std::string* dL;
                const std::string* dR;
                get_neighbors(na.first, node_neighbors.at(it->second.id), node_angles.at(it->second.id), &dL, &dR);
                Rectangle rect;
                if (!Rectangle::from_line(
                    rect,
                    nodes.at(*aR).position,
                    nodes.at(*aL).position,
                    nodes.at(na.first).position,
                    nodes.at(it->second.id).position,
                    nodes.at(*dL).position,
                    nodes.at(*dR).position,
                    *aR == na.first ? it->second.width : node_neighbors.at(*aR).at(na.first).width,
                    *aL == na.first ? it->second.width : node_neighbors.at(*aL).at(na.first).width,
                    it->second.width,
                    *dL == na.first ? it->second.width : node_neighbors.at(*dL).at(it->second.id).width,
                    *dR == na.first ? it->second.width : node_neighbors.at(*dR).at(it->second.id).width))
                {
                    std::cerr << "Error triangulating street nodes " <<
                        it->second.id << " <-> " << na.first << ", (" <<
                        nodes.at(it->second.id).position << ") <-> (" << nodes.at(na.first).position << ")" << std::endl;
                } else {
                    // Way length is used to get connected street textures where possible.
                    auto len0 = node_way_info.find(na.first);
                    auto len1 = node_way_info.find(it->second.id);
                    auto& street_lst = it->second.road_type == RoadType::STREET ? tl_street : tl_path;
                    auto& curb_lst = it->second.road_type == RoadType::STREET ? tl_curb_street : tl_curb_path;
                    bool with_b_height_binding;
                    bool with_c_height_binding;
                    {
                        const auto& tags = nodes.at(na.first).tags;
                        with_b_height_binding = with_height_bindings && (tags.find("bind_height") == tags.end() || tags.at("bind_height") == "yes");
                    }
                    {
                        const auto& tags = nodes.at(it->second.id).tags;
                        with_c_height_binding = with_height_bindings && (tags.find("bind_height") == tags.end() || tags.at("bind_height") == "yes");
                    }
                    if ((len0 != node_way_info.end()) &&
                        (len1 != node_way_info.end()))
                    {
                        rect.draw_z0(street_lst, height_bindings, na.first, it->second.id, len0->second.color, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, -curb_alpha, curb_alpha, false, with_b_height_binding, with_c_height_binding);
                        if (curb_alpha != 1) {
                            rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, len0->second.color, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, -1, -curb_alpha, true, with_b_height_binding, with_c_height_binding);
                            rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, len0->second.color, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, curb_alpha, 1, false, with_b_height_binding, with_c_height_binding);
                        }
                    } else {
                        float len = std::sqrt(sum(squared(nodes.at(na.first).position - nodes.at(it->second.id).position)));
                        rect.draw_z0(street_lst, height_bindings, na.first, it->second.id, way_color, 0, len / scale * uv_scale, -curb_alpha, curb_alpha, false, with_b_height_binding, with_c_height_binding);
                        if (curb_alpha != 1) {
                            rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, way_color, 0, len / scale * uv_scale, -1, -curb_alpha, true, with_b_height_binding, with_c_height_binding);
                            rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, way_color, 0, len / scale * uv_scale, curb_alpha, 1, false, with_b_height_binding, with_c_height_binding);
                        }
                        // rect.draw_z0(tl_street_crossing);
                    }
                    if (na.second.size() >= 3) {
                        CurbedStreet c1{rect, -curb_alpha, curb_alpha};
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{it->first, 1}, c1.s00));
                        if (curb_alpha != 1) {
                            CurbedStreet c0{rect, -1, -curb_alpha};
                            CurbedStreet c2{rect, curb_alpha, 1};
                            node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{it->first, 2}, c0.s00));
                            node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{it->first, 0}, c2.s00));
                        }
                    }
                    const std::map<std::string, NeighborWay>& nn = node_neighbors.at(it->second.id);
                    if (nn.size() >= 3) {
                        CurbedStreet c1{rect, -curb_alpha, curb_alpha};
                        // Left and right are swapped for the neighbor, so we use p11_ instead of p10_.
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{angle: nn.at(na.first).angle, curb: 1}, c1.s11));
                        if (curb_alpha != 1) {
                            CurbedStreet c0{rect, -1, -curb_alpha};
                            CurbedStreet c2{rect, curb_alpha, 1};
                            node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{angle: nn.at(na.first).angle, curb: 0}, c0.s11));
                            node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{angle: nn.at(na.first).angle, curb: 2}, c2.s11));
                        }
                    }
                    if (!street_lights.empty()) {
                        float radius = 10 * scale;
                        auto add_distant_point = [&](const FixedArray<float, 2>& p) {
                            bool p_found = false;
                            bvh.visit(BoundingSphere(p, radius), [&p_found](const std::string&, bool){p_found=true;});
                            if (!p_found) {
                                bvh.insert(p, "", true);
                                add_parsed_resource_name(p, street_lights(), 1, resource_instance_positions, object_resource_descriptors, hitboxes);
                            }
                        };
                        add_distant_point(rect.p00_);
                        add_distant_point(rect.p11_);
                    }
                    //for(float a = 0.1; a < 0.91; a += 0.4) {
                    //    auto p = a * rect.p00_ + (1 - a) * rect.p10_;
                    //    street_light_positions.push_back(std::make_pair(FixedArray<float, 3>{p(0), p(1), 0}, "bgrass"));
                    //}
                }
            }
        }
    }

    for(const auto& nh : node_hole_contours) {
        Array<FixedArray<float, 2>> hv{ArrayShape{nh.second.size()}};
        {
            size_t i = 0;
            for(const auto& h : nh.second) {
                hv(i++) = h.second;
            }
        }
        RoadType road_type = RoadType::PATH;
        for(const auto& a : node_angles.at(nh.first)) {
            if (a.second.road_type == RoadType::STREET) {
                road_type = RoadType::STREET;
            }
        }
        auto& crossings = (road_type == RoadType::STREET)
            ? tl_street_crossing
            : tl_path_crossing;
        // A single triangle does not work with curbs when an angle is ~90°
        if ((nh.second.size() == 3) && (curb_alpha == 1)) {
            crossings.draw_triangle_wo_normals(
                FixedArray<float, 3>{hv(0)(0), hv(0)(1), 0},
                FixedArray<float, 3>{hv(1)(0), hv(1)(1), 0},
                FixedArray<float, 3>{hv(2)(0), hv(2)(1), 0});
        } else if (nh.second.size() >= 3) {
            const FixedArray<float, 2>& center = mean(hv);
            for(size_t i = 0; i < hv.length(); ++i) {
                size_t j = (i + 1) % hv.length();
                crossings.draw_triangle_wo_normals(
                    FixedArray<float, 3>{hv(i)(0), hv(i)(1), 0},
                    FixedArray<float, 3>{hv(j)(0), hv(j)(1), 0},
                    FixedArray<float, 3>{center(0), center(1), 0});
            }
            if (with_height_bindings) {
                auto& tags = nodes.at(nh.first).tags;
                if (tags.find("bind_height") == tags.end() || tags.at("bind_height") == "yes") {
                    height_bindings[OrderableFixedArray{center}].insert(nh.first);
                }
            }
        }
    }

    for(TriangleList* l : std::list<TriangleList*>{&tl_street_crossing, &tl_path_crossing}) {
        for(auto& t : l->triangles_) {
            t(0).color = way_color;
            t(1).color = way_color;
            t(2).color = way_color;
            t(0).uv = {t(0).position(0) / scale * uv_scale, t(0).position(1) / scale * uv_scale};
            t(1).uv = {t(1).position(0) / scale * uv_scale, t(1).position(1) / scale * uv_scale};
            t(2).uv = {t(2).position(0) / scale * uv_scale, t(2).position(1) / scale * uv_scale};
        }
    }
}

void Mlib::raise_streets(
    TriangleList& tl_street_crossing,
    TriangleList& tl_path_crossing,
    TriangleList& tl_street,
    TriangleList& tl_path,
    TriangleList& tl_curb_street,
    TriangleList& tl_curb_path,
    TriangleList& tl_terrain,
    float scale,
    float amount)
{
    std::set<OrderableFixedArray<float, 3>> lowered_nodes;
    auto ins = [&](const std::list<FixedArray<ColoredVertex, 3>>& l){
        for(const auto& n : l) {
            lowered_nodes.insert(OrderableFixedArray{n(0).position});
            lowered_nodes.insert(OrderableFixedArray{n(1).position});
            lowered_nodes.insert(OrderableFixedArray{n(2).position});
        }
    };
    ins(tl_street_crossing.triangles_);
    ins(tl_path_crossing.triangles_);
    ins(tl_street.triangles_);
    ins(tl_path.triangles_);
    auto lower = [&](std::list<FixedArray<ColoredVertex, 3>>& l){
        for(auto& n : l) {
            for(auto& v : n.flat_iterable()) {
                if (lowered_nodes.find(OrderableFixedArray{v.position}) != lowered_nodes.end()) {
                    v.position(2) += scale * amount;
                }
            }
        }
    };
    lower(tl_street_crossing.triangles_);
    lower(tl_path_crossing.triangles_);
    lower(tl_street.triangles_);
    lower(tl_path.triangles_);
    lower(tl_curb_street.triangles_);
    lower(tl_curb_path.triangles_);
    lower(tl_terrain.triangles_);
}

class PTri {
public:
    p2t::Point* operator () (size_t i) const {
        return v->GetPoint(i);
    }
    bool operator < (const PTri& other) const {
        return v < other.v;
    }
private:
    p2t::Triangle* v;
};

BoundingInfo::BoundingInfo(
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::map<std::string, Node>& nodes,
    float border_width)
: border_width{border_width}
{
    boundary_min = fixed_full<float, 2>(INFINITY);
    boundary_max = fixed_full<float, 2>(-INFINITY);
    if (bounding_contour.empty()) {
        for(const auto& n : nodes) {
            boundary_min = minimum(boundary_min, n.second.position);
            boundary_max = maximum(boundary_max, n.second.position);
        }
    } else {
        for(const auto& p : bounding_contour) {
            boundary_min = minimum(boundary_min, p);
            boundary_max = maximum(boundary_max, p);
        }
    }
}

void Mlib::add_street_steiner_points(
    std::list<SteinerPointInfo>& steiner_points,
    const std::list<FixedArray<ColoredVertex, 3>>& triangles,
    const BoundingInfo& bounding_info,
    float scale,
    const std::vector<float>& steiner_point_distances_road,
    const std::vector<float>& steiner_point_distances_steiner)
{
    if (!steiner_point_distances_road.empty()) {
        typedef FixedArray<FixedArray<float, 2>, 3> Triangle2d;
        // for(float f = 0.01; f < 2; f += 0.01) {
        //     Bvh<float, Triangle2d, 2> bvh{{f, f}, 10};
        //     for(const auto& t : triangles) {
        //         Triangle2d tri{
        //             FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
        //             FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
        //             FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
        //         bvh.insert(tri, "", tri);
        //     }
        //     std::cerr << "f " << f << " search_time " << bvh.search_time() << std::endl;
        // }
        Bvh<float, Triangle2d, 2> bvh{{0.1, 0.1}, 10};
        for(const auto& t : triangles) {
            Triangle2d tri{
                FixedArray<float, 2>{t(0).position(0), t(0).position(1)},
                FixedArray<float, 2>{t(1).position(0), t(1).position(1)},
                FixedArray<float, 2>{t(2).position(0), t(2).position(1)}};
            if (triangle_is_right_handed(tri(0), tri(1), tri(2))) {
                bvh.insert(tri, "", tri);
            }
        }
        Interp<float> interp{steiner_point_distances_road, steiner_point_distances_steiner, OutOfRangeBehavior::CLAMP};
        // std::cerr << "search_time " << bvh.search_time() << std::endl;
        float dist0 = (*std::min_element(steiner_point_distances_steiner.begin(), steiner_point_distances_steiner.end())) * scale;
        float dist1 = 0;
        for(float v : steiner_point_distances_steiner) {
            if (v != INFINITY) {
                dist1 = std::max(dist1, v * scale);
            }
        }
        size_t ix = 0;
        NormalRandomNumberGenerator<float> rng2{0, 0, 1.2};
        for(float x = bounding_info.boundary_min(0) + bounding_info.border_width / 2; x < bounding_info.boundary_max(0) - bounding_info.border_width / 2; x += dist0) {
            size_t iy = 0;
            for(float y = bounding_info.boundary_min(1) + bounding_info.border_width / 2; y < bounding_info.boundary_max(1) - bounding_info.border_width / 2; y += dist0) {
                float min_distance = INFINITY;
                FixedArray<float, 2> pt{x + rng2() * scale, y + rng2() * scale};
                bvh.visit(BoundingSphere<float, 2>(pt, dist1), [&min_distance, &pt](const std::string& category, const Triangle2d& tri) {
                    min_distance = std::min(min_distance, distance_point_to_triangle(pt, tri(0), tri(1), tri(2)));
                });
                if (min_distance > 0) {
                    float dist = interp(min_distance / scale);
                    if (dist != INFINITY) {
                        size_t refinement = std::max<size_t>(1, (dist * scale) / dist0);
                        bool is_included = (ix % refinement == 0) && (iy % refinement == 0);
                        if (is_included) {
                            steiner_points.push_back(SteinerPointInfo{
                                .position = {pt(0), pt(1), 0},
                                .type = SteinerPointType::STREET_NEIGHBOR,
                                .distance_to_road = min_distance});
                        }
                    }
                }
                ++iy;
            }
            ++ix;
        }
    }
}

void Mlib::triangulate_terrain_or_ceilings(
    TriangleList& tl_terrain,
    const BoundingInfo& bounding_info,
    const std::list<SteinerPointInfo>& steiner_points,
    const std::vector<FixedArray<float, 2>>& bounding_contour,
    const std::list<FixedArray<ColoredVertex, 3>>& hole_triangles,
    const std::map<std::string, Node>& nodes,
    float scale,
    float uv_scale,
    float z)
{
    p2t::Point p00{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p01{bounding_info.boundary_min(0) - bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    p2t::Point p10{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_min(1) - bounding_info.border_width};
    p2t::Point p11{bounding_info.boundary_max(0) + bounding_info.border_width, bounding_info.boundary_max(1) + bounding_info.border_width};
    std::vector<p2t::Point> p2t_bounding_nodes;
    p2t_bounding_nodes.reserve(bounding_contour.size());
    std::vector<p2t::Point*> p2t_bounding_contour;
    p2t_bounding_contour.reserve(bounding_contour.size());
    for(const auto& p : bounding_contour) {
        p2t_bounding_nodes.push_back(p2t::Point{p(0), p(1)});
        p2t_bounding_contour.push_back(&p2t_bounding_nodes.back());
    }
    auto final_bounding_contour = bounding_contour.empty()
        ? std::vector<p2t::Point*>{&p00, &p01, &p11, &p10}
        : p2t_bounding_contour;
    p2t::CDT cdt{final_bounding_contour};
    //PolygonDrawer pd;
    //pd.draw_line(p00, p01, 10);
    //pd.draw_line(p01, p11, 10);
    //pd.draw_line(p11, p10, 10);
    //pd.draw_line(p10, p00, 10);
    //p2t::CDT cdt{pd.points_};
    if (false) {
        std::vector<p2t::Point> p2t_nodes;
        p2t_nodes.reserve(3 * hole_triangles.size());
        for(const auto& t : hole_triangles) {
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(0).position(0), t(0).position(1)}) != p2t_nodes.end()) continue;
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(1).position(0), t(1).position(1)}) != p2t_nodes.end()) continue;
            if (std::find(p2t_nodes.begin(), p2t_nodes.end(), p2t::Point{t(2).position(0), t(2).position(1)}) != p2t_nodes.end()) continue;
            p2t_nodes.push_back(p2t::Point{t(0).position(0), t(0).position(1)});
            p2t_nodes.push_back(p2t::Point{t(1).position(0), t(1).position(1)});
            p2t_nodes.push_back(p2t::Point{t(2).position(0), t(2).position(1)});
            cdt.AddHole(std::vector<p2t::Point*>{
                &p2t_nodes[p2t_nodes.size() - 3],
                &p2t_nodes[p2t_nodes.size() - 2],
                &p2t_nodes[p2t_nodes.size() - 1]});
        }
    }
    auto hole_contours = find_contours(hole_triangles);

    std::vector<std::vector<p2t::Point>> p2t_hole_nodes;
    p2t_hole_nodes.reserve(hole_contours.size());
    for(const std::list<FixedArray<float, 3>>& c : hole_contours) {
        p2t_hole_nodes.push_back(std::vector<p2t::Point>());
        auto& pts = p2t_hole_nodes.back();
        pts.reserve(c.size());
        std::vector<p2t::Point*> hole_contour;
        hole_contour.reserve(c.size());
        // size_t i = 0;
        for(const auto& p : c) {
            pts.push_back(p2t::Point{p(0), p(1)});
            hole_contour.push_back(&pts.back());
            // draw_node(triangles, FixedArray<float, 2>{p(0), p(1)}, 0.1 * float(i++) / c.size());
        }
        cdt.AddHole(hole_contour);
    }
    std::list<p2t::Point> p2t_grid_nodes;
    for(const auto& p : steiner_points) {
        p2t_grid_nodes.push_back(p2t::Point{p.position(0), p.position(1)});
        cdt.AddPoint(&p2t_grid_nodes.back());
    }
    std::vector<p2t::Point> p2t_triangle_centers;
    p2t_triangle_centers.reserve(hole_triangles.size());
    std::set<p2t::Point*> p2t_hole_triangle_centers_set;
    for(const auto& t : hole_triangles) {
        auto center = (t(0).position + t(1).position + t(2).position) / 3.f;
        p2t_triangle_centers.push_back(p2t::Point{center(0), center(1)});
        cdt.AddPoint(&*p2t_triangle_centers.rbegin());
        p2t_hole_triangle_centers_set.insert(&*p2t_triangle_centers.rbegin());
    }
    //triangles.clear();
    cdt.Triangulate();
    std::list<p2t::Triangle*> tris;
    if (hole_contours.empty()) {
        auto tris0 = cdt.GetTriangles();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
    } else {
        auto tris0 = cdt.GetMap();
        tris.insert(tris.end(), tris0.begin(), tris0.end());
        std::list<PTri>& wrapped_tris = reinterpret_cast<std::list<PTri>&>(tris);
        delete_triangles_outside_contour(final_bounding_contour, wrapped_tris);
    }
    for(const auto& t : tris) {
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(0)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(1)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        if (p2t_hole_triangle_centers_set.find(t->GetPoint(2)) != p2t_hole_triangle_centers_set.end()) {
            continue;
        }
        tl_terrain.draw_triangle_wo_normals(
            {float(t->GetPoint(0)->x), t->GetPoint(0)->y, z * scale},
            {float(t->GetPoint(1)->x), t->GetPoint(1)->y, z * scale},
            {float(t->GetPoint(2)->x), t->GetPoint(2)->y, z * scale},
            terrain_color,
            terrain_color,
            terrain_color,
            {float(t->GetPoint(0)->x) / scale * uv_scale, t->GetPoint(0)->y / scale * uv_scale},
            {float(t->GetPoint(1)->x) / scale * uv_scale, t->GetPoint(1)->y / scale * uv_scale},
            {float(t->GetPoint(2)->x) / scale * uv_scale, t->GetPoint(2)->y / scale * uv_scale});
    }
}

struct NodeHeight {
    float height;
    float smooth_height;
};

struct NeighborWeight {
    std::string id;
    float weight;
};

void Mlib::apply_height_map(
    std::list<FixedArray<float, 3>*>& in_vertices,
    std::set<const FixedArray<float, 3>*>& vertices_to_delete,
    const Array<float>& heightmap,
    const FixedArray<float, 2, 3>& normalization_matrix,
    float scale,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    const std::map<OrderableFixedArray<float, 2>, std::set<std::string>>& height_bindings,
    float street_node_smoothness)
{
    std::map<std::string, NodeHeight> node_height;
    if (street_node_smoothness != 0) {
        std::map<std::string, std::list<NeighborWeight>> node_neighbors;
        for(const auto& w : ways) {
            for(auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                auto s = it;
                ++s;
                if (s != w.second.nd.end()) {
                    float weight = 1 / std::sqrt(sum(squared(nodes.at(*it).position - nodes.at(*s).position)));
                    node_neighbors[*s].push_back({.id = *it, .weight = weight});
                    node_neighbors[*it].push_back({.id = *s, .weight = weight});
                }
            }
        }
        for(const auto& n : node_neighbors) {
            FixedArray<float, 2> p = dot1d(normalization_matrix, homogenized_3(nodes.at(n.first).position));
            float z;
            if (bilinear_grayscale_interpolation((1 - p(1)) * (heightmap.shape(0) - 1), p(0) * (heightmap.shape(1) - 1), heightmap, z)) {
                node_height[n.first] = {
                    .height = z,
                    .smooth_height = z};
            }
        }
        for(size_t i = 0; i < 50; ++i) {
            // std::cerr << i << " " << node_neighbors.size() << std::endl;
            for(const auto& n : node_neighbors) {
                auto hit = node_height.find(n.first);
                if (hit != node_height.end()) {
                    float mean_height = 0;
                    float sum_weights = 0;
                    for(const auto& b : n.second) {
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
    std::map<OrderableFixedArray<float, 2>, std::list<FixedArray<float, 3>*>> vertex_instances_map;
    for (FixedArray<float, 3>* iv : in_vertices) {
        vertex_instances_map[OrderableFixedArray<float, 2>{(*iv)(0), (*iv)(1)}].push_back(iv);
    }
    for(auto& position : vertex_instances_map) {
        FixedArray<float, 2> vc;
        auto it = height_bindings.find(OrderableFixedArray<float, 2>{position.first(0), position.first(1)});
        if ((it != height_bindings.end()) && (it->second.size() == 1)) {
            if (auto hit = node_height.find(*it->second.begin()); hit != node_height.end()) {
                for(auto& pc : position.second) {
                    (*pc)(2) += hit->second.smooth_height * scale;
                }
                continue;
            }
            vc = nodes.at(*it->second.begin()).position;
        } else {
            vc = {position.first(0), position.first(1)};
        }
        FixedArray<float, 2> p = dot1d(normalization_matrix, homogenized_3(vc));
        float z;
        if (!bilinear_grayscale_interpolation((1 - p(1)) * (heightmap.shape(0) - 1), p(0) * (heightmap.shape(1) - 1), heightmap, z)) {
            // std::cerr << "Height out of bounds." << std::endl;
            for(auto& pc : position.second) {
                if (!vertices_to_delete.insert(pc).second) {
                    throw std::runtime_error("Could not insert vertex to delete");
                }
            }
        } else {
            for(auto& pc : position.second) {
                (*pc)(2) += z * scale;
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
    NormalRandomNumberGenerator<float> rng{0, 1, 0.2};
    for(const auto& p : steiner_points) {
        if ((p.type == SteinerPointType::STREET_NEIGHBOR) &&
            !std::isnan(p.distance_to_road) &&
            ((p.distance_to_road > dmin * scale) &&
             (p.distance_to_road < dmax * scale)))
        {
            add_parsed_resource_name(p.position, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
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
    NormalRandomNumberGenerator<float> rng{0, 1, 0.2};
    NormalRandomNumberGenerator<float> rng2{0, 0, 1.2};
    size_t gid = 0;
    for(auto& t : triangles.triangles_) {
        float dist_a = std::sqrt(std::min(sum(squared(t(1).position - t(0).position)), sum(squared(t(2).position - t(0).position))));
        float dist_b = std::sqrt(std::min(sum(squared(t(1).position - t(0).position)), sum(squared(t(2).position - t(1).position))));
        for(float a = 0.01; a < 0.99; a += distance * scale / dist_a) {
            for(float b = 0.01; b < 1 - a; b += distance * scale / dist_b) {
                float aa = a + rng2() * distance * scale / dist_a;
                float bb = b + rng2() * distance * scale / dist_b;
                float c = 1 - aa - bb;
                if (c < 0 || aa < 0 || bb < 0) {
                    continue;
                }
                auto p = aa * t(0).position + bb * t(1).position + c * t(2).position;
                ++gid;
                add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            }
        }
    }
}

void Mlib::add_trees_to_forest_outlines(
    std::map<std::string, std::list<ResourceInstanceDescriptor>>& resource_instance_positions,
    std::list<ObjectResourceDescriptor>& object_resource_descriptors,
    std::map<std::string, std::list<FixedArray<float, 3>>>& hitboxes,
    std::list<SteinerPointInfo>& steiner_points,
    ResourceNameCycle& rnc,
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways,
    float tree_distance,
    float tree_inwards_distance,
    float scale)
{
    NormalRandomNumberGenerator<float> rng{0, 1, 0.2};
    NormalRandomNumberGenerator<float> rng2{0, 0, 1.2};
    // size_t rid = 0;
    for(const auto& w : ways) {
        const auto& tags = w.second.tags;
        if ((tags.find("landuse") != tags.end() && tags.at("landuse") == "forest") ||
            (tags.find("natural") != tags.end() && tags.at("natural") == "wood"))
        {
            float area = compute_area(w.second.nd, nodes, scale);
            for(auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
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
                for(float a = 0.1; a < 0.91; a += tree_distance * scale / len) {
                    float aa = a + rng2() * tree_distance * scale / len;
                    if (aa < 0 || aa > 0.91) {
                        continue;
                    }
                    FixedArray<float, 2> p = (aa * p0 + (1 - aa) * p1) + tree_inwards_distance * scale * n * sign(area);
                    add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
                    // object_resource_descriptors.push_back({
                    //     position: FixedArray<float, 3>{p(0), p(1), 0},
                    //     name: rnc(),
                    //     scale: rng()});
                    // if ((rid++) % 4 == 0) {
                    steiner_points.push_back({
                        .position = {p(0), p(1), 0},
                        .type = SteinerPointType::FOREST_OUTLINE,
                        .distance_to_road = NAN});
                    // }
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
    for(const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("raceway") != tags.end() && tags.at("raceway") == "yes")
        {
            auto sw = smooth_way(nodes, w.second.nd, scale, raceway_beacon_distance);
            for(const auto p : sw) {
                street_light_positions.push_back({FixedArray<float, 3>{p(0), p(1), 0}, "raceway_beacon", 1});
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
//     for(float f : linspace<float>(tree_inwards_distance, tree_inwards_distance + 5.f, 3).flat_iterable()) {
//         for(const auto& w : ways) {
//             const auto& tags = w.second.tags;
//             if ((tags.find("landuse") != tags.end() && (tags.at("landuse") == "farmland" || tags.at("landuse") == "meadow")))
//             {
//                 float area = compute_area(w.second.nd, nodes, scale);
//                 for(auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
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
//                     for(float a = 0.1; a < 0.91; a += tree_distance * scale / len) {
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
    const std::map<std::string, Node>& nodes,
    float scale)
{
    NormalRandomNumberGenerator<float> rng{0, 1, 0.2};
    for(const auto& n : nodes) {
        const auto& tags = n.second.tags;
        if (tags.find("natural") != tags.end() && tags.at("natural") == "tree") {
            const auto& p = n.second.position;
            add_parsed_resource_name(p, rnc(), rng(), resource_instance_positions, object_resource_descriptors, hitboxes);
            steiner_points.push_back({
                .position = {p(0), p(1), 0},
                .type = SteinerPointType::TREE_NODE,
                .distance_to_road = NAN});
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
    for(auto& t : ground_triangles.triangles_) {
        ++tid;
        tls.push_back(std::make_shared<TriangleList>("binary_vegetation_old", material));
        float veg_size;
        switch (tid % 10) {
            case 0:
                tls.back()->material_.texture_descriptor.color = grass_texture;
                veg_size = 1;
                break;
            case 2:
                tls.back()->material_.texture_descriptor.color = tree_texture;
                veg_size = 5;
                break;
            case 4:
                tls.back()->material_.texture_descriptor.color = tree_texture_2;
                veg_size = 5;
                break;
            default:
                continue;
                // throw std::runtime_error("Internal error");
        }
        auto center = (t(0).position + t(1).position + t(2).position) / 3.f;
        tls.back()->draw_rectangle_wo_normals(
            center + FixedArray<float, 3>{-scale * veg_size, 0, 0},
            center + FixedArray<float, 3>{scale * veg_size, 0, 0},
            center + FixedArray<float, 3>{scale * veg_size, 0, 2 * scale * veg_size},
            center + FixedArray<float, 3>{-scale * veg_size, 0, 2 * scale * veg_size},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1});
        tls.back()->draw_rectangle_wo_normals(
            center + FixedArray<float, 3>{0, -scale * veg_size, 0},
            center + FixedArray<float, 3>{0, scale * veg_size, 0},
            center + FixedArray<float, 3>{0, scale * veg_size, 2 * scale * veg_size},
            center + FixedArray<float, 3>{0, -scale * veg_size, 2 * scale * veg_size},
            {1, 1, 1},
            {1, 1, 1},
            {1, 1, 1});
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
    for(auto& t : triangles) {
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
    for(auto& b : buildings) {
        b.area = compute_area(b.way.nd, nodes, scale);
    }
}

void Mlib::draw_building_walls(
    std::list<std::shared_ptr<TriangleList>>& tls,
    std::list<SteinerPointInfo>& steiner_points,
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
    for(const auto& bu : buildings) {
        ++bid;
        tls.push_back(std::make_shared<TriangleList>("building_walls", material));
        tls.back()->material_.texture_descriptor.color = facade_textures.at(bid % facade_textures.size());
        auto sw = smooth_way(nodes, bu.way.nd, scale, max_width);
        for(auto it = sw.begin(); it != sw.end(); ++it) {
            auto s = it;
            ++s;
            if (s != sw.end()) {
                const auto& p0 = bu.area > 0 ? *s : *it;
                const auto& p1 = bu.area > 0 ? *it : *s;
                float width = std::sqrt(sum(squared(p0 - p1)));
                float height = (bu.building_top - bu.building_bottom) * scale;
                steiner_points.push_back({
                    .position = {p0(0), p0(1), 0},
                    .type = SteinerPointType::WALL,
                    .distance_to_road = NAN});
                // some buildings are clock-wise, others counter-clock-wise
                tls.back()->draw_rectangle_wo_normals(
                    {p1(0), p1(1), bu.building_bottom * scale},
                    {p0(0), p0(1), bu.building_bottom * scale},
                    {p0(0), p0(1), bu.building_top * scale},
                    {p1(0), p1(1), bu.building_top * scale},
                    building_color,
                    building_color,
                    building_color,
                    building_color,
                    {0, 0},
                    {width / scale * uv_scale, 0},
                    {width / scale * uv_scale, height / scale * uv_scale},
                    {0, height / scale * uv_scale});
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
    for(const auto& p : nodes) {
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

ResourceNameCycle::ResourceNameCycle(const SceneNodeResources& resources, const std::vector<std::string>& names)
: rng0_{1, 0, names.size() - 1},
  rng_{2}
{
    const std::regex re{"^(.*?)\\(p=([\\d+.e-]+)(?:,hitbox=(\\w+))?\\)$"};
    names_.reserve(names.size());
    for(const std::string& name : names) {
        std::smatch match;
        if (std::regex_match(name, match, re)) {
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

const ParsedResourceName& ResourceNameCycle::operator() () {
    if (names_.empty()) {
        throw std::runtime_error("ResourceNameCycle called with empty names");
    }
    while(true) {
        const ParsedResourceName& prn = names_[rng0_()];
        if (prn.probability != 1) {
            if (rng_() < prn.probability) {
                return prn;
            }
        } else {
            return prn;
        }
    }
}

bool ResourceNameCycle::empty() const {
    return names_.empty();
}
