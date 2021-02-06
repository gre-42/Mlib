#include "Draw_Streets.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Rectangle.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Parsed_Resource_Name.hpp>
#include <Mlib/Render/Resources/Resource_Instance_Descriptor.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <Mlib/Strings/String.hpp>
#include <regex>

using namespace Mlib;

namespace Mlib {

struct AngleCurb {
    float angle;
    int curb;
    auto operator <=> (const AngleCurb& other) const = default;
};

struct NodeWaypoint {
    std::string node;
    FixedArray<float, 2> position;
};

struct HoleWaypoint {
    std::list<NodeWaypoint> in;
    std::list<NodeWaypoint> out;
};

}

DrawStreets::DrawStreets(const DrawStreetsInput& in)
: DrawStreetsInput{in}
{
    check_curb_validity(in.curb_alpha, in.curb2_alpha);
    initialize_arrays();
    calculate_neighbors();
    draw_streets();
    draw_holes();
}

DrawStreets::~DrawStreets()
{}

void DrawStreets::initialize_arrays() {
    for (const auto& n : nodes) {
        node_angles.insert(std::make_pair(n.first, std::map<float, AngleWay>()));
        node_neighbors.insert(std::make_pair(n.first, std::map<std::string, NeighborWay>()));
        node_hole_contours.insert(std::make_pair(n.first, std::map<AngleCurb, FixedArray<float, 2>>()));
        if (driving_direction == DrivingDirection::LEFT || driving_direction == DrivingDirection::RIGHT) {
            node_hole_waypoints_street.insert(std::make_pair(n.first, HoleWaypoint()));
            node_hole_waypoints_sidewalk.insert(std::make_pair(n.first, HoleWaypoint()));
        } else if (driving_direction != DrivingDirection::CENTER) {
            throw std::runtime_error("Unknown driving direction");
        }
    }
}

void DrawStreets::calculate_neighbors() {
    DECLARE_REGEX(name_re, name_pattern);
    std::set<std::string> node_no_way_length;
    for (const auto& w : ways) {
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
            for (const auto& x : excluded_highway_tags) {
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
            if (!name_pattern.empty() && ((tags.find("name") == tags.end()) || !Mlib::re::regex_match(tags.at("name"), name_re))) {
                continue;
            }
            if (tags.find("area") != tags.end() && tags.at("area") == "yes") {
                continue;
            }
            float width = scale * parse_meters(tags, "width", default_street_width);
            RoadType road_type = path_tags.contains(tags.at("highway")) ? RoadType::PATH : RoadType::STREET;
            float way_length = 0;
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                if (node_no_way_length.find(*it) == node_no_way_length.end()) {
                    if (node_way_info.find(*it) != node_way_info.end()) {
                        node_no_way_length.insert(*it);
                        node_way_info.erase(*it);
                    } else {
                        node_way_info.insert(std::make_pair(*it, NodeWayInfo{.way_length = way_length, .color = color}));
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
}

void DrawStreets::draw_streets() {
    Bvh<float, bool, 2> street_light_bvh{{0.1f, 0.1f}, 10};

    // Compute rectangles and holes for each pair of connected nodes.
    // To avoid duplicates, the computations are done at each lexicographically
    // smaller node.
    for (const auto& na : node_angles) {
        for (auto it = na.second.begin(); it != na.second.end(); ++it) {
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
                if (sum(squared(nodes.at(na.first).position - nodes.at(it->second.id).position)) < squared(0.1 * scale))
                {
                    std::cerr << "Skipping street because it is too short. " <<
                        it->second.id << " <-> " << na.first << ", (" <<
                        nodes.at(it->second.id).position << ") <-> (" << nodes.at(na.first).position << ")" << std::endl;
                    continue;
                }
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
                    continue;
                }
                size_t nlanes;
                float lane_alpha;
                float sidewalk_alpha0 = 0.75f * curb2_alpha + 0.25f * 1.f;
                float sidewalk_alpha1 = 0.25f * curb2_alpha + 0.75f * 1.f;
                float car_width = 2;
                if (it->second.width < 4 * car_width * scale) {
                    nlanes = 2;
                    // alpha is in [-1 .. +1]
                    lane_alpha = 0.5f * curb_alpha;
                } else {
                    nlanes = 4;
                    // alpha is in [-1 .. +1]
                    lane_alpha = 0.25f * curb_alpha;
                }
                if (driving_direction == DrivingDirection::CENTER) {
                    way_point_edges_1_lane_street.push_back({na.first, it->second.id});
                } else if (driving_direction == DrivingDirection::LEFT) {
                    auto add = [&rect](float start, float stop, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& lanes){
                        CurbedStreet c5{rect, start, stop};
                        lanes.push_back({
                            FixedArray<float, 3>{c5.s00(0), c5.s00(1), 0.f},
                            FixedArray<float, 3>{c5.s10(0), c5.s10(1), 0.f}});
                        lanes.push_back({
                            FixedArray<float, 3>{c5.s11(0), c5.s11(1), 0.f},
                            FixedArray<float, 3>{c5.s01(0), c5.s01(1), 0.f}});
                    };
                    add(-lane_alpha, lane_alpha, way_point_edges_2_lanes[WayPointLocation::STREET]);
                    if (curb2_alpha != 1) {
                        add(sidewalk_alpha0, sidewalk_alpha1, way_point_edges_2_lanes[WayPointLocation::SIDEWALK]);
                        add(-sidewalk_alpha1, -sidewalk_alpha0, way_point_edges_2_lanes[WayPointLocation::SIDEWALK]);
                    }
                } else if (driving_direction == DrivingDirection::RIGHT) {
                    auto add = [&rect](float start, float stop, std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& lanes){
                        CurbedStreet c5{rect, start, stop};
                        lanes.push_back({
                            FixedArray<float, 3>{c5.s10(0), c5.s10(1), 0.f},
                            FixedArray<float, 3>{c5.s00(0), c5.s00(1), 0.f}});
                        lanes.push_back({
                            FixedArray<float, 3>{c5.s01(0), c5.s01(1), 0.f},
                            FixedArray<float, 3>{c5.s11(0), c5.s11(1), 0.f}});
                    };
                    add(-lane_alpha, lane_alpha, way_point_edges_2_lanes[WayPointLocation::STREET]);
                    if (curb2_alpha != 1) {
                        add(sidewalk_alpha0, sidewalk_alpha1, way_point_edges_2_lanes[WayPointLocation::SIDEWALK]);
                        add(-sidewalk_alpha1, -sidewalk_alpha0, way_point_edges_2_lanes[WayPointLocation::SIDEWALK]);
                    }
                } else {
                    throw std::runtime_error("Unknown driving direction");
                }
                {
                    auto add = [this, &rect](
                        float start,
                        float stop,
                        WayPointLocation location,
                        size_t nlanes)
                    {
                        CurbedStreet c1{rect, start, stop};
                        street_rectangles.push_back(StreetRectangle{
                            .location = location,
                            .nlanes = nlanes,
                            .rectangle = FixedArray<FixedArray<float, 3>, 2, 2>{
                                FixedArray<float, 3>{c1.s00(0), c1.s00(1), 0.f},
                                FixedArray<float, 3>{c1.s01(0), c1.s01(1), 0.f},
                                FixedArray<float, 3>{c1.s10(0), c1.s10(1), 0.f},
                                FixedArray<float, 3>{c1.s11(0), c1.s11(1), 0.f}}});
                    };
                    add(-curb_alpha, curb_alpha, WayPointLocation::STREET, nlanes);
                    if (curb2_alpha != 1) {
                        add(-1, -curb2_alpha, WayPointLocation::SIDEWALK, 2);
                        add(curb2_alpha, 1, WayPointLocation::SIDEWALK, 2);
                    }
                }
                // Way length is used to get connected street textures where possible.
                auto len0 = node_way_info.find(na.first);
                auto len1 = node_way_info.find(it->second.id);
                auto& street_lst = it->second.road_type == RoadType::STREET ? tl_street : tl_path;
                auto& curb_lst = it->second.road_type == RoadType::STREET ? tl_curb_street : tl_curb_path;
                auto& curb2_lst = it->second.road_type == RoadType::STREET ? tl_curb2_street : tl_curb2_path;
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
                    rect.draw_z0(street_lst, height_bindings, na.first, it->second.id, len0->second.color, 0, 1, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, -curb_alpha, curb_alpha, false, with_b_height_binding, with_c_height_binding);
                    if (curb_alpha != 1) {
                        rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, len0->second.color, 0, curb_uv_x, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, -curb2_alpha, -curb_alpha, true, with_b_height_binding, with_c_height_binding);
                        rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, len0->second.color, 0, curb_uv_x, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, curb_alpha, curb2_alpha, false, with_b_height_binding, with_c_height_binding);
                    }
                    if (curb2_alpha != 1) {
                        rect.draw_z0(curb2_lst, height_bindings, na.first, it->second.id, len0->second.color, 0, curb2_uv_x, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, -1, -curb2_alpha, true, with_b_height_binding, with_c_height_binding);
                        rect.draw_z0(curb2_lst, height_bindings, na.first, it->second.id, len0->second.color, 0, curb2_uv_x, len0->second.way_length / scale * uv_scale, len1->second.way_length / scale * uv_scale, curb2_alpha, 1, false, with_b_height_binding, with_c_height_binding);
                    }
                } else {
                    float len = std::sqrt(sum(squared(nodes.at(na.first).position - nodes.at(it->second.id).position)));
                    rect.draw_z0(street_lst, height_bindings, na.first, it->second.id, way_color, 0, 1, 0, len / scale * uv_scale, -curb_alpha, curb_alpha, false, with_b_height_binding, with_c_height_binding);
                    if (curb_alpha != 1) {
                        rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, way_color, 0, curb_uv_x, 0, len / scale * uv_scale, -curb2_alpha, -curb_alpha, true, with_b_height_binding, with_c_height_binding);
                        rect.draw_z0(curb_lst, height_bindings, na.first, it->second.id, way_color, 0, curb_uv_x, 0, len / scale * uv_scale, curb_alpha, curb2_alpha, false, with_b_height_binding, with_c_height_binding);
                    }
                    if (curb2_alpha != 1) {
                        rect.draw_z0(curb2_lst, height_bindings, na.first, it->second.id, way_color, 0, curb2_uv_x, 0, len / scale * uv_scale, -1, -curb2_alpha, true, with_b_height_binding, with_c_height_binding);
                        rect.draw_z0(curb2_lst, height_bindings, na.first, it->second.id, way_color, 0, curb2_uv_x, 0, len / scale * uv_scale, curb2_alpha, 1, false, with_b_height_binding, with_c_height_binding);
                    }
                    // rect.draw_z0(tl_street_crossing);
                }
                if (na.second.size() >= 3) {
                    {
                        CurbedStreet c0{rect, -curb_alpha, curb_alpha};
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{.angle = it->first, .curb = 0}, c0.s00));
                    }
                    if (curb_alpha != 1) {
                        CurbedStreet cN{rect, -curb2_alpha, -curb_alpha};
                        CurbedStreet cP{rect, curb_alpha, curb2_alpha};
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{.angle = it->first, .curb = +1}, cN.s00));
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{.angle = it->first, .curb = -1}, cP.s00));
                    }
                    if (curb2_alpha != 1) {
                        CurbedStreet cN{rect, -1, -curb2_alpha};
                        CurbedStreet cP{rect, curb2_alpha, 1};
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{.angle = it->first, .curb = +2}, cN.s00));
                        node_hole_contours.at(na.first).insert(std::make_pair(AngleCurb{.angle = it->first, .curb = -2}, cP.s00));
                    }
                    if (driving_direction == DrivingDirection::LEFT) {
                        auto add = [&rect, &na, &it](float start, float stop, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                            CurbedStreet c5{rect, start, stop};
                            node_hole_waypoints.at(na.first).out.push_back({it->second.id, c5.s00});
                            node_hole_waypoints.at(na.first).in.push_back({it->second.id, c5.s01});
                        };
                        add(-lane_alpha, lane_alpha, node_hole_waypoints_street);
                        if (curb2_alpha != 1) {
                            add(sidewalk_alpha0, sidewalk_alpha1, node_hole_waypoints_sidewalk);
                            add(-sidewalk_alpha1, -sidewalk_alpha0, node_hole_waypoints_sidewalk);
                        }
                    } else if (driving_direction == DrivingDirection::RIGHT) {
                        auto add = [&rect, &na, &it](float start, float stop, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                            CurbedStreet c5{rect, start, stop};
                            node_hole_waypoints.at(na.first).in.push_back({it->second.id, c5.s00});
                            node_hole_waypoints.at(na.first).out.push_back({it->second.id, c5.s01});
                        };
                        add(-lane_alpha, lane_alpha, node_hole_waypoints_street);
                        if (curb2_alpha != 1) {
                            add(sidewalk_alpha0, sidewalk_alpha1, node_hole_waypoints_sidewalk);
                            add(-sidewalk_alpha1, -sidewalk_alpha0, node_hole_waypoints_sidewalk);
                        }
                    } else if (driving_direction != DrivingDirection::CENTER) {
                        throw std::runtime_error("Unknown driving direction");
                    }
                }
                const std::map<std::string, NeighborWay>& nn = node_neighbors.at(it->second.id);
                if (nn.size() >= 3) {
                    {
                        CurbedStreet c0{rect, -curb_alpha, curb_alpha};
                        // Left and right are swapped for the neighbor, so we use p11_ instead of p10_.
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{.angle = nn.at(na.first).angle, .curb = 0}, c0.s11));
                    }
                    if (curb_alpha != 1) {
                        CurbedStreet cN{rect, -curb2_alpha, -curb_alpha};
                        CurbedStreet cP{rect, curb_alpha, curb2_alpha};
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{.angle = nn.at(na.first).angle, .curb = -1}, cN.s11));
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{.angle = nn.at(na.first).angle, .curb = +1}, cP.s11));
                    }
                    if (curb2_alpha != 1) {
                        CurbedStreet cN{rect, -1, -curb2_alpha};
                        CurbedStreet cP{rect, curb2_alpha, 1};
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{.angle = nn.at(na.first).angle, .curb = -2}, cN.s11));
                        node_hole_contours.at(it->second.id).insert(std::make_pair(AngleCurb{.angle = nn.at(na.first).angle, .curb = +2}, cP.s11));
                    }
                    if (driving_direction == DrivingDirection::LEFT) {
                        auto add = [&rect, &na, &it](float start, float stop, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                            CurbedStreet c5{rect, start, stop};
                            node_hole_waypoints.at(it->second.id).out.push_back({na.first, c5.s11});
                            node_hole_waypoints.at(it->second.id).in.push_back({na.first, c5.s10});
                        };
                        add(-lane_alpha, lane_alpha, node_hole_waypoints_street);
                        if (curb2_alpha != 1) {
                            add(sidewalk_alpha0, sidewalk_alpha1, node_hole_waypoints_sidewalk);
                            add(-sidewalk_alpha1, -sidewalk_alpha0, node_hole_waypoints_sidewalk);
                        }
                    } else if (driving_direction == DrivingDirection::RIGHT) {
                        auto add = [&rect, &na, &it](float start, float stop, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                            CurbedStreet c5{rect, start, stop};
                            node_hole_waypoints.at(it->second.id).in.push_back({na.first, c5.s11});
                            node_hole_waypoints.at(it->second.id).out.push_back({na.first, c5.s10});
                        };
                        add(-lane_alpha, lane_alpha, node_hole_waypoints_street);
                        if (curb2_alpha != 1) {
                            add(sidewalk_alpha0, sidewalk_alpha1, node_hole_waypoints_sidewalk);
                            add(-sidewalk_alpha1, -sidewalk_alpha0, node_hole_waypoints_sidewalk);
                        }
                    } else if (driving_direction != DrivingDirection::CENTER) {
                        throw std::runtime_error("Unknown driving direction");
                    }
                }
                if (!street_lights.empty()) {
                    float radius = 10 * scale;
                    auto add_distant_point = [&](const FixedArray<float, 2>& p) {
                        bool p_found = false;
                        street_light_bvh.visit(BoundingSphere(p, radius), [&p_found](bool){p_found=true;});
                        if (!p_found) {
                            street_light_bvh.insert(p, true);
                            add_parsed_resource_name(p, street_lights(), 1, resource_instance_positions, object_resource_descriptors, hitboxes);
                        }
                    };
                    add_distant_point(rect.p00_);
                    add_distant_point(rect.p11_);
                }
                //for (float a = 0.1; a < 0.91; a += 0.4) {
                //    auto p = a * rect.p00_ + (1 - a) * rect.p10_;
                //    street_light_positions.push_back(std::make_pair(FixedArray<float, 3>{p(0), p(1), 0}, "bgrass"));
                //}
            }
        }
    }
}

void DrawStreets::draw_holes() {
    if (driving_direction == DrivingDirection::LEFT ||
        driving_direction == DrivingDirection::RIGHT)
    {
        auto connect = [](
            std::map<std::string, HoleWaypoint>& node_hole_waypoints,
            std::list<std::pair<FixedArray<float, 3>, FixedArray<float, 3>>>& way_point_edges_2_lanes)
        {
            for (const auto& nw : node_hole_waypoints) {
                for (const auto& x : nw.second.in) {
                    for (const auto& y : nw.second.out) {
                        if (x.node == y.node) {
                            continue;
                        }
                        way_point_edges_2_lanes.push_back({
                            FixedArray<float, 3>{x.position(0), x.position(1), 0.f},
                            FixedArray<float, 3>{y.position(0), y.position(1), 0.f}});
                    }
                }
            }
        };
        connect(node_hole_waypoints_street, way_point_edges_2_lanes[WayPointLocation::STREET]);
        connect(node_hole_waypoints_sidewalk, way_point_edges_2_lanes[WayPointLocation::SIDEWALK]);
    } else if (driving_direction != DrivingDirection::CENTER) {
        throw std::runtime_error("Only 1 or 2 lanes are supported");
    }
    for (const auto& nh : node_hole_contours) {
        Array<FixedArray<float, 2>> hv{ArrayShape{nh.second.size()}};
        {
            size_t i = 0;
            for (const auto& h : nh.second) {
                if (curb_alpha != 1) {
                    if (h.first.curb == 0 || h.first.curb == -1) {
                        hv(i++) = h.second;
                    }
                } else {
                    hv(i++) = h.second;
                }
            }
            hv.reshape(ArrayShape{i});
        }
        RoadType road_type = RoadType::PATH;
        for (const auto& a : node_angles.at(nh.first)) {
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
                FixedArray<float, 3>{hv(0)(0), hv(0)(1), 0.f},
                FixedArray<float, 3>{hv(1)(0), hv(1)(1), 0.f},
                FixedArray<float, 3>{hv(2)(0), hv(2)(1), 0.f});
        } else if (nh.second.size() >= 3) {
            // Draw center fan
            {
                const FixedArray<float, 2>& center = mean(hv);
                for (size_t i = 0; i < hv.length(); ++i) {
                    size_t j = (i + 1) % hv.length();
                    crossings.draw_triangle_wo_normals(
                        FixedArray<float, 3>{hv(i)(0), hv(i)(1), 0.f},
                        FixedArray<float, 3>{hv(j)(0), hv(j)(1), 0.f},
                        FixedArray<float, 3>{center(0), center(1), 0.f});
                }
                if (with_height_bindings) {
                    auto& tags = nodes.at(nh.first).tags;
                    if (tags.find("bind_height") == tags.end() || tags.at("bind_height") == "yes") {
                        height_bindings[OrderableFixedArray{center}].insert(nh.first);
                    }
                }
            }
            // Draw corners
            if (curb_alpha != 1) {
                std::vector<float> angles;
                {
                    std::set<float> angles_set;
                    for (const auto& e : nh.second) {
                        angles_set.insert(e.first.angle);
                    }
                    angles = std::vector<float>(angles_set.begin(), angles_set.end());
                }
                for (size_t i = 0; i < angles.size(); ++i) {
                    size_t j = (i + 1) % angles.size();
                    auto draw_rect = [&](TriangleList& tl, int curb0, int curb1, int curb2, int curb3, float uv_x) {
                        auto p00 = nh.second.at(AngleCurb{angles[i], curb0});
                        auto p10 = nh.second.at(AngleCurb{angles[i], curb1});
                        auto p11 = nh.second.at(AngleCurb{angles[j], curb2});
                        auto p01 = nh.second.at(AngleCurb{angles[j], curb3});
                        // float len = std::sqrt(sum(squared((p00 + p10) / 2.f - (p01 + p11) / 2.f)));
                        float len = std::sqrt(sum(squared(p00 - p01)));
                        // std::cerr << std::sqrt(sum(squared(p00 - p01))) << " " << std::sqrt(sum(squared(p10 - p11))) << std::endl;
                        float f = uv_x;
                        float g = len / scale * uv_scale;
                        tl.draw_rectangle_wo_normals(
                            FixedArray<float, 3>{p00(0), p00(1), 0.f},
                            FixedArray<float, 3>{p10(0), p10(1), 0.f},
                            FixedArray<float, 3>{p11(0), p11(1), 0.f},
                            FixedArray<float, 3>{p01(0), p01(1), 0.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 2>{0.f, 0.f},
                            FixedArray<float, 2>{f  , 0.f},
                            FixedArray<float, 2>{f  , g  },
                            FixedArray<float, 2>{0.f, g  });
                    };
                    auto draw_triangle = [&](TriangleList& tl, int curb0, int curb1, int curb2, float uv_x) {
                        auto p00 = nh.second.at(AngleCurb{angles[i], curb0});
                        auto p10 = nh.second.at(AngleCurb{angles[i], curb1});
                        auto p01 = nh.second.at(AngleCurb{angles[j], curb2});
                        float len = std::sqrt(sum(squared(p00 - p01)));
                        float f = uv_x;
                        float g = len / scale * uv_scale;
                        float h = g / 2;
                        tl.draw_triangle_wo_normals(
                            FixedArray<float, 3>{p00(0), p00(1), 0.f},
                            FixedArray<float, 3>{p10(0), p10(1), 0.f},
                            FixedArray<float, 3>{p01(0), p01(1), 0.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 3>{1.f, 1.f, 1.f},
                            FixedArray<float, 2>{0.f, 0.f},
                            FixedArray<float, 2>{f  , h  },
                            FixedArray<float, 2>{0.f, g  });
                    };
                    if (curb2_alpha != 1) {
                        draw_rect(tl_curb_street, 0, 1, -2, -1, curb_uv_x);
                        draw_triangle(tl_curb2_street, 1, 2, -2, curb2_uv_x);
                    } else {
                        draw_triangle(tl_curb_street, 0, 1, -1, curb_uv_x);
                    }
                }
            }
        }
    }

    for (TriangleList* l : std::list<TriangleList*>{&tl_street_crossing, &tl_path_crossing}) {
        for (auto& t : l->triangles_) {
            t(0).color = way_color;
            t(1).color = way_color;
            t(2).color = way_color;
            t(0).uv = {t(0).position(0) / scale * uv_scale, t(0).position(1) / scale * uv_scale};
            t(1).uv = {t(1).position(0) / scale * uv_scale, t(1).position(1) / scale * uv_scale};
            t(2).uv = {t(2).position(0) / scale * uv_scale, t(2).position(1) / scale * uv_scale};
        }
    }
}
