#include "Draw_Streets.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Geometry/Mesh/Animated_Colored_Vertex_Arrays.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Entrance_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Node_Height_Binding.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Rectangle_2D.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Triangle_Lists.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Racing_Line_Bvh.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Resource_Name_Cycle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Connection_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Road_Type.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Street_Rectangle.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Styled_Road.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Vertex_Way_Point.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Batch_Resource_Instantiator.hpp>
#include <Mlib/Scene_Graph/Driving_Direction.hpp>
#include <Mlib/Scene_Graph/Parsed_Resource_Name.hpp>
#include <Mlib/Scene_Graph/Scene_Node_Resources.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <Mlib/Strings/From_Number.hpp>
#include <Mlib/Strings/String.hpp>

using namespace Mlib;

namespace Mlib {

std::pair<FixedArray<double, 3>, FixedArray<double, 3>> o23(const std::pair<FixedArray<double, 2>, FixedArray<double, 2>>& edge) {
    return std::pair<FixedArray<double, 3>, FixedArray<double, 3>>{
        { edge.first(0), edge.first(1), 0. },
        { edge.second(0), edge.second(1), 0. } };
}

std::pair<FixedArray<double, 3>, FixedArray<double, 3>> o23(const FixedArray<double, 2>& a, const FixedArray<double, 2>& b) {
    return std::pair<FixedArray<double, 3>, FixedArray<double, 3>>{
        { a(0), a(1), 0. },
        { b(0), b(1), 0. } };
}

struct AngleWay {
    std::string neighbor_id;
    float width;
    unsigned int nlanes;
    RoadType road_type;
    int layer;
    std::string way_id;
    bool neighbor_is_second;
};

struct NeighborWay {
    float angle;
    float width;
};

struct NodeWayInfo {
    std::string way_id;
    float way_length;
    float layer;  // Has type float to support NAN
};

struct AngleCurb {
    float angle;
    int curb;
    std::partial_ordering operator <=> (const AngleCurb& other) const = default;
};

struct NodeHoleWaypoint {
    std::string node;
    std::pair<float, float> alpha;
    std::pair<FixedArray<double, 2>, FixedArray<double, 2>> edge;
};

struct HoleWaypoint {
    std::list<NodeHoleWaypoint> in;
    std::list<NodeHoleWaypoint> out;
};

struct WayInfo {
    float curb_alpha;
    float curb2_alpha;
    FixedArray<bool, 3> roads_delete;
    FixedArray<FixedArray<bool, 3>, 2> roads_delete_side;
    FixedArray<FixedArray<float, 3>, 3> colors;
    std::string model;
};

struct NodeHoleVertex {
    FixedArray<double, 2> position;
    std::string way_id;
};

}

static void get_neighbors(
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
        *l = &angles.rbegin()->second.neighbor_id;
    } else {
        auto itL = it;
        *l = &(--itL)->second.neighbor_id;
    }
    auto itR = it;
    ++itR;
    if (itR == angles.end()) {
        *r = &angles.begin()->second.neighbor_id;
    } else {
        *r = &itR->second.neighbor_id;
    }
}

DrawStreets::DrawStreets(const DrawStreetsInput& in)
: DrawStreetsInput{in}
{
    check_curb_validity(in.curb_alpha_, in.curb2_alpha_);
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
        node_hole_contours.insert(std::make_pair(n.first, std::map<AngleCurb, NodeHoleVertex>()));
        air_support_node_hole_contours.insert(std::make_pair(n.first, std::map<AngleCurb, NodeHoleVertex>()));
        tunnel_node_hole_contours.insert(std::make_pair(n.first, std::map<AngleCurb, NodeHoleVertex>()));
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
    for (const auto& w : ways) {
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
            if (only_raceways_and_walls &&
                    !tags.contains("highway", "raceway") &&
                    !tags.contains("raceway", "yes") &&
                    !tags.contains("highway", "wall"))
            {
                continue;
            }
            if (!name_pattern.empty() && ((tags.find("name") == tags.end()) || !Mlib::re::regex_match(tags.at("name"), name_re))) {
                continue;
            }
            if (tags.contains("area", "yes")) {
                continue;
            }
            float width = ((tags.find("lanes") != tags.end()) && (tags.find("width") == tags.end()))
                ? scale * default_lane_width * safe_stou(tags.at("lanes"))
                : scale * parse_meters(tags, "width", default_street_width);
            unsigned int nlanes;
            if (tags.find("lanes") != tags.end()) {
                nlanes = safe_stou(tags.at("lanes"));
            } else {
                float car_width = 3;
                if (width < 2 * car_width * scale) {
                    nlanes = 1;
                } else if (width < 4 * car_width * scale) {
                    nlanes = 2;
                } else {
                    nlanes = 4;
                }
            }
            RoadType road_type = RoadType::STREET;
            if (path_tags.contains(tags.at("highway")) ||
                ((tags.find("lanes") != tags.end()) && tags.at("lanes") == "1"))
            {
                road_type = RoadType::PATH;
            }
            if (tags.at("highway") == "wall")
            {
                road_type = RoadType::WALL;
            }
            int layer = (tags.find("layer") == tags.end()) ? 0 : safe_stoi(tags.at("layer"));
            if ((layer != 0) && !layer_heights.is_within_range((float)layer)) {
                continue;
            }
            std::string model = parse_string(tags, "model", "");
            float way_length = 0;
            for (auto it = w.second.nd.begin(); it != w.second.nd.end(); ++it) {
                if (nodes.find(*it) == nodes.end()) {
                    throw std::runtime_error("Way " + w.first + ": Could not find node with ID " + *it);
                }
                {
                    auto nwi = node_way_info.find(*it);
                    if (nwi != node_way_info.end()) {
                        nwi->second.way_length = NAN;
                        if (!std::isnan(nwi->second.layer) &&
                            (nwi->second.layer != (float)layer))
                        {
                            nwi->second.layer = NAN;
                        }
                    } else {
                        node_way_info.insert(std::make_pair(*it, NodeWayInfo{
                            .way_id = w.first,
                            .way_length = way_length,
                            .layer = (float)layer}));
                    }
                }
                auto s = it;
                ++s;
                if (s != w.second.nd.end()) {
                    if (nodes.find(*s) == nodes.end()) {
                        throw std::runtime_error("Way " + w.first + ": Could not find node with ID " + *s);
                    }
                    FixedArray<double, 2> dir = nodes.at(*it).position - nodes.at(*s).position;
                    float angle0 = std::atan2(dir(1), dir(0));
                    float angle1 = std::atan2(-dir(1), -dir(0));
                    node_angles.at(*it).insert({angle0, AngleWay{*s, width, nlanes, road_type, layer, w.first, true}});
                    node_angles.at(*s).insert({angle1, AngleWay{*it, width, nlanes, road_type, layer, w.first, false}});
                    node_neighbors.at(*it).insert({*s, NeighborWay{angle0, width}});
                    node_neighbors.at(*s).insert({*it, NeighborWay{angle1, width}});
                    way_segments.push_back({nodes.at(*s).position, nodes.at(*it).position});
                    way_length += std::sqrt(sum(squared(nodes.at(*s).position - nodes.at(*it).position)));
                }
            }
            if (!way_infos.insert({
                w.first,
                WayInfo{                
                    .curb_alpha = parse_float(tags, "curb_alpha", curb_alpha_),
                    .curb2_alpha = parse_float(tags, "curb2_alpha", curb2_alpha_),
                    .roads_delete = {
                        false,
                        parse_bool(tags, "curb_delete", false),
                        parse_bool(tags, "curb2_delete", false)},
                    .roads_delete_side = {
                        FixedArray<bool, 3>{
                            false,
                            parse_bool(tags, "curb_right_delete", false),
                            parse_bool(tags, "curb2_right_delete", false)},
                        FixedArray<bool, 3>{
                            false,
                            parse_bool(tags, "curb_left_delete", false),
                            parse_bool(tags, "curb2_left_delete", false)},
                    },
                    .colors = {
                        parse_color(tags, "color", way_color),
                        parse_color(tags, "curb_color", curb_color_),
                        parse_color(tags, "curb2_color", way_color)},
                    .model = model}}).second)
            {
                throw std::runtime_error("Could not insert way");
            }
            check_curb_validity(way_infos.at(w.first).curb_alpha, way_infos.at(w.first).curb2_alpha);
        }
    }
}

void DrawStreets::draw_streets() {
    Bvh<double, bool, 2> street_light_bvh{{0.1, 0.1}, 10};

    // Compute rectangles and holes for each pair of connected nodes.
    // The "neighbor_is_second" field is used to avoid duplicates.
    for (const auto& na : node_angles) {
        for (const auto& it : na.second) {
            if (it.second.neighbor_is_second) {
                // node index: na.first
                // neighbor index: it.second.neighbor_id
                // angle of neighbor: it.first
                // angle at neighbor: node_neighbors.at(it.second).at(na.first)
                const std::string* aL;
                const std::string* aR;
                get_neighbors(it.second.neighbor_id, node_neighbors.at(na.first), na.second, &aL, &aR);
                const std::string* dL;
                const std::string* dR;
                get_neighbors(na.first, node_neighbors.at(it.second.neighbor_id), node_angles.at(it.second.neighbor_id), &dL, &dR);
                if (sum(squared(nodes.at(na.first).position - nodes.at(it.second.neighbor_id).position)) < squared(0.1 * scale))
                {
                    std::cerr << "Skipping street because it is too short. " <<
                        it.second.neighbor_id << " <-> " << na.first << ", (" <<
                        nodes.at(it.second.neighbor_id).position << ") <-> (" << nodes.at(na.first).position << ")" << std::endl;
                    continue;
                }
                OsmRectangle2D rect;
                if (!OsmRectangle2D::from_line(
                    rect,
                    nodes.at(*aR).position,
                    nodes.at(*aL).position,
                    nodes.at(na.first).position,
                    nodes.at(it.second.neighbor_id).position,
                    nodes.at(*dL).position,
                    nodes.at(*dR).position,
                    *aR == na.first ? it.second.width : node_neighbors.at(*aR).at(na.first).width,
                    *aL == na.first ? it.second.width : node_neighbors.at(*aL).at(na.first).width,
                    it.second.width,
                    it.second.width,
                    *dL == na.first ? it.second.width : node_neighbors.at(*dL).at(it.second.neighbor_id).width,
                    *dR == na.first ? it.second.width : node_neighbors.at(*dR).at(it.second.neighbor_id).width))
                {
                    std::cerr << "Error triangulating street nodes " <<
                        it.second.neighbor_id << " <-> " << na.first << ", (" <<
                        nodes.at(it.second.neighbor_id).position << ") <-> (" << nodes.at(na.first).position << ")" << std::endl;
                    continue;
                }
                draw_streets_draw_ways(
                    rect,
                    na.first,
                    it.second);
                draw_streets_find_hole_contours(
                    rect,
                    na.first,
                    it.second,
                    it.first);
                {
                    const auto& wi = way_infos.at(it.second.way_id);
                    float lane_shift;
                    if (it.second.nlanes <= 2) {
                        // alpha is in [-1 .. +1]
                        lane_shift = 0.f;
                    } else {
                        // alpha is in [-1 .. +1]
                        lane_shift = 0.125f;
                    }
                    if (it.second.road_type != RoadType::WALL) {
                        draw_streets_add_waypoints(
                            rect,
                            wi.curb_alpha,
                            wi.curb2_alpha,
                            it.second.nlanes,
                            lane_shift,
                            na.first,
                            it.second);
                        draw_streets_find_hole_waypoints(
                            rect,
                            na.first,
                            it.second.neighbor_id,
                            wi.curb_alpha,
                            wi.curb2_alpha,
                            lane_shift);
                    }
                }
                if ((it.second.road_type != RoadType::WALL) && (!street_lights.empty())) {
                    double radius = 10 * scale;
                    auto add_distant_point = [&](const FixedArray<double, 2>& p) {
                        bool p_found = !street_light_bvh.visit(AxisAlignedBoundingBox{ p, radius }, [&p_found](bool){return false;});
                        if (!p_found) {
                            street_light_bvh.insert(p, true);
                            bri.add_parsed_resource_name(p, 0.f, street_lights(), 0.f, 1.f);
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
            const std::map<std::string, HoleWaypoint>& node_hole_waypoints,
            std::list<std::pair<StreetWayPoint, StreetWayPoint>>& way_point_edge_descriptors)
        {
            for (const auto& nw : node_hole_waypoints) {
                for (const auto& x : nw.second.in) {
                    for (const auto& y : nw.second.out) {
                        if (x.node == y.node) {
                            continue;
                        }
                        way_point_edge_descriptors.push_back({
                            StreetWayPoint{.alpha = x.alpha, .edge = o23(x.edge)},
                            StreetWayPoint{.alpha = y.alpha, .edge = o23(y.edge)}});
                    }
                }
            }
        };
        connect(node_hole_waypoints_street, way_point_edge_descriptors[WayPointLocation::STREET]);
        connect(node_hole_waypoints_sidewalk, way_point_edge_descriptors[WayPointLocation::SIDEWALK]);
    } else if (driving_direction != DrivingDirection::CENTER) {
        throw std::runtime_error("Only 1 or 2 lanes are supported");
    }
    auto draw_air_holes = [this](const auto& hole_contours, auto& hole_triangles) {
        for (const auto& nh : hole_contours) {
            Array<NodeHoleVertex> hv{ArrayShape{nh.second.size()}};
            {
                size_t i = 0;
                for (const auto& h : nh.second) {
                    hv(i++) = h.second;
                }
                hv.reshape(ArrayShape{i});
            }
            if (nh.second.size() == 0) {
                // do nothing
            } else if (nh.second.size() == 3) {
                hole_triangles->draw_triangle_wo_normals(
                    FixedArray<double, 3>{hv(0).position(0), hv(0).position(1), 0.},
                    FixedArray<double, 3>{hv(1).position(0), hv(1).position(1), 0.},
                    FixedArray<double, 3>{hv(2).position(0), hv(2).position(1), 0.},
                    way_infos.at(hv(0).way_id).colors(0),
                    way_infos.at(hv(1).way_id).colors(0),
                    way_infos.at(hv(2).way_id).colors(0));
            } else if (nh.second.size() > 3) {
                // Draw center fan
                FixedArray<double, 2> center = mean(hv TEMPLATE applied<FixedArray<double, 2>>([](auto& v){return v.position;}));
                FixedArray<float, 3> center_color = mean(hv TEMPLATE applied<FixedArray<float, 3>>([&](auto& v){return way_infos.at(v.way_id).colors(0);}));
                for (size_t i = 0; i < hv.length(); ++i) {
                    size_t j = (i + 1) % hv.length();
                    hole_triangles->draw_triangle_wo_normals(
                        FixedArray<double, 3>{hv(i).position(0), hv(i).position(1), 0.f},
                        FixedArray<double, 3>{hv(j).position(0), hv(j).position(1), 0.f},
                        FixedArray<double, 3>{center(0), center(1), 0.f},
                        way_infos.at(hv(i).way_id).colors(0),
                        way_infos.at(hv(j).way_id).colors(0),
                        center_color);
                }
            } else {
                throw std::runtime_error("Unexpected air hole size");
            }
        }
    };
    draw_air_holes(air_support_node_hole_contours, air_triangles.tl_air_support);
    draw_air_holes(tunnel_node_hole_contours, air_triangles.tl_tunnel_crossing);
    for (const auto& nh : node_hole_contours) {
        if (nh.second.empty()) {
            continue;
        }
        Array<NodeHoleVertex> hv{ArrayShape{nh.second.size()}};
        {
            size_t i = 0;
            for (const auto& h : nh.second) {
                if (curb_alpha_ != 1) {
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
        OsmTriangleLists* tlist2 = &air_triangles;
        for (const auto& a : node_angles.at(nh.first)) {
            if (a.second.road_type == RoadType::STREET) {
                road_type = RoadType::STREET;
            }
            if (a.second.layer == 0) {
                tlist2 = &ground_triangles;
            }
        }
        auto sit = uv_scales.find(road_type);
        if (sit == uv_scales.end()) {
            throw std::runtime_error("Could not find uv_scale for " + road_type_to_string(road_type));
        }
        float uv_scale = sit->second;
        float curb_alpha = NAN;
        float curb2_alpha = NAN;
        for (const auto& e : nh.second) {
            const auto& wi = way_infos.at(e.second.way_id);
            if (std::isnan(curb_alpha)) {
                curb_alpha = wi.curb_alpha;
            } else if ((curb_alpha != 1) != (wi.curb_alpha != 1)) {
                throw std::runtime_error("Incompatible curb alpha");
            }
            if (std::isnan(curb2_alpha)) {
                curb2_alpha = wi.curb2_alpha;
            } else if ((curb2_alpha != 1) != (wi.curb2_alpha != 1)) {
                throw std::runtime_error("Incompatible curb2 alpha");
            }
        }
        auto& crossings = *tlist2->tl_street_crossing[road_type];
        // A single triangle does not work with curbs when an angle is ~90??
        if ((nh.second.size() == 3) && (curb_alpha_ == 1)) {
            crossings.draw_triangle_wo_normals(
                FixedArray<double, 3>{hv(0).position(0), hv(0).position(1), 0.f},
                FixedArray<double, 3>{hv(1).position(0), hv(1).position(1), 0.f},
                FixedArray<double, 3>{hv(2).position(0), hv(2).position(1), 0.f},
                way_infos.at(hv(0).way_id).colors(0),
                way_infos.at(hv(1).way_id).colors(0),
                way_infos.at(hv(2).way_id).colors(0));
        } else if (nh.second.size() >= 3) {
            // Draw center fan
            {
                FixedArray<double, 2> center = mean(hv TEMPLATE applied<FixedArray<double, 2>>([](auto& v){return v.position;}));
                FixedArray<float, 3> center_color = mean(hv TEMPLATE applied<FixedArray<float, 3>>([&](auto& v){return way_infos.at(v.way_id).colors(0);}));
                for (size_t i = 0; i < hv.length(); ++i) {
                    size_t j = (i + 1) % hv.length();
                    crossings.draw_triangle_wo_normals(
                        FixedArray<double, 3>{hv(i).position(0), hv(i).position(1), 0.},
                        FixedArray<double, 3>{hv(j).position(0), hv(j).position(1), 0.},
                        FixedArray<double, 3>{center(0), center(1), 0.f},
                        way_infos.at(hv(i).way_id).colors(0),
                        way_infos.at(hv(j).way_id).colors(0),
                        center_color);
                }
                if (with_height_bindings && !nodes.at(nh.first).tags.contains("bind_height", "no")) {
                    node_height_bindings[OrderableFixedArray{ center }] = nh.first;
                }
            }
            // Draw corners
            if (curb_alpha_ != 1) {
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
                    auto draw_rect = [&](TriangleList<double>& tl, int curb0, int curb1, int curb2, int curb3, const FixedArray<float, 2>& uv, size_t road_id) {
                        const auto& p00 = nh.second.at(AngleCurb{angles[i], curb0});
                        const auto& p10 = nh.second.at(AngleCurb{angles[i], curb1});
                        const auto& p11 = nh.second.at(AngleCurb{angles[j], curb2});
                        const auto& p01 = nh.second.at(AngleCurb{angles[j], curb3});
                        if (way_infos.at(p00.way_id).roads_delete(road_id) &&
                            way_infos.at(p10.way_id).roads_delete(road_id) &&
                            way_infos.at(p11.way_id).roads_delete(road_id) &&
                            way_infos.at(p01.way_id).roads_delete(road_id))
                        {
                            return;
                        }
                        // float len = std::sqrt(sum(squared((p00 + p10) / 2.f - (p01 + p11) / 2.f)));
                        float len = std::sqrt(sum(squared(p00.position - p01.position)));
                        // std::cerr << std::sqrt(sum(squared(p00 - p01))) << " " << std::sqrt(sum(squared(p10 - p11))) << std::endl;
                        float f = uv(0);
                        float g = uv(1) * len / scale * uv_scale;
                        tl.draw_rectangle_wo_normals(
                            FixedArray<double, 3>{p00.position(0), p00.position(1), 0.f},
                            FixedArray<double, 3>{p10.position(0), p10.position(1), 0.f},
                            FixedArray<double, 3>{p11.position(0), p11.position(1), 0.f},
                            FixedArray<double, 3>{p01.position(0), p01.position(1), 0.f},
                            way_infos.at(p00.way_id).colors(road_id),
                            way_infos.at(p10.way_id).colors(road_id),
                            way_infos.at(p11.way_id).colors(road_id),
                            way_infos.at(p01.way_id).colors(road_id),
                            FixedArray<float, 2>{0.f, 0.f},
                            FixedArray<float, 2>{f  , 0.f},
                            FixedArray<float, 2>{f  , g  },
                            FixedArray<float, 2>{0.f, g  });
                    };
                    auto draw_triangle = [&](TriangleList<double>& tl, int curb0, int curb1, int curb2, const FixedArray<float, 2>& uv, size_t road_id) {
                        auto p00 = nh.second.at(AngleCurb{angles[i], curb0});
                        auto p10 = nh.second.at(AngleCurb{angles[i], curb1});
                        auto p01 = nh.second.at(AngleCurb{angles[j], curb2});
                        if (way_infos.at(p00.way_id).roads_delete(road_id) &&
                            way_infos.at(p10.way_id).roads_delete(road_id) &&
                            way_infos.at(p01.way_id).roads_delete(road_id))
                        {
                            return;
                        }
                        float len = std::sqrt(sum(squared(p00.position - p01.position)));
                        float f = uv(0);
                        float g = uv(1) * len / scale * uv_scale;
                        float h = g / 2;
                        tl.draw_triangle_wo_normals(
                            FixedArray<double, 3>{p00.position(0), p00.position(1), 0.f},
                            FixedArray<double, 3>{p10.position(0), p10.position(1), 0.f},
                            FixedArray<double, 3>{p01.position(0), p01.position(1), 0.f},
                            way_infos.at(p00.way_id).colors(road_id),
                            way_infos.at(p10.way_id).colors(road_id),
                            way_infos.at(p01.way_id).colors(road_id),
                            FixedArray<float, 2>{0.f, 0.f},
                            FixedArray<float, 2>{f  , h  },
                            FixedArray<float, 2>{0.f, g  });
                    };
                    if (curb2_alpha != 1) {
                        draw_rect(*tlist2->tl_street_curb[RoadType::STREET], 0, 1, -2, -1, curb_uv, 1);
                        draw_triangle(*tlist2->tl_street_curb2[RoadType::STREET], 1, 2, -2, curb2_uv, 2);
                    } else {
                        // "if (curb_alpha != 1)" already checked above.
                        draw_triangle(*tlist2->tl_street_curb[RoadType::STREET], 0, 1, -1, curb_uv, 1);
                    }
                }
            }
        }
    }

    for (std::list<std::shared_ptr<TriangleList<double>>>& l : std::vector<std::list<std::shared_ptr<TriangleList<double>>>>{
        ground_triangles.tls_crossing_only(),
        air_triangles.tls_crossing_only()})
    {
        for (const auto& l2 : l) {
            for (auto& t : l2->triangles_) {
                // t(0).color = way_color;
                // t(1).color = way_color;
                // t(2).color = way_color;
                t(0).uv = {float(t(0).position(0) / scale * uv_scale_crossings), float(t(0).position(1) / scale * uv_scale_crossings)};
                t(1).uv = {float(t(1).position(0) / scale * uv_scale_crossings), float(t(1).position(1) / scale * uv_scale_crossings)};
                t(2).uv = {float(t(2).position(0) / scale * uv_scale_crossings), float(t(2).position(1) / scale * uv_scale_crossings)};
            }
        }
    }
    // for (TriangleList* l : std::vector<TriangleList*>{
    //     air_triangles.tl_air_support.get(),
    //     air_triangles.tl_tunnel_crossing.get()})
    // {
    //     for (auto& t : l->triangles_) {
    //         t(0).color = way_color;
    //         t(1).color = way_color;
    //         t(2).color = way_color;
    //     }
    // }
}

void DrawStreets::draw_streets_add_waypoints(
    const OsmRectangle2D& rect,
    float curb_alpha,
    float curb2_alpha,
    unsigned int nlanes,
    float lane_shift,
    const std::string& node_id,
    const AngleWay& angle_way)
{
    if (driving_direction == DrivingDirection::CENTER) {
        CurbedStreet c5{rect, -curb_alpha, curb_alpha};
        way_point_edge_descriptors[WayPointLocation::STREET].push_back({
            StreetWayPoint{.alpha{0.5f, 0.5f}, .edge{o23(c5.s00, c5.s01)}},
            StreetWayPoint{.alpha{0.5f, 0.5f}, .edge{o23(c5.s10, c5.s11)}}});
    } else if (driving_direction == DrivingDirection::LEFT) {
        auto add = [&rect](float start, float stop, float shift, std::list<std::pair<StreetWayPoint, StreetWayPoint>>& lanes){
            CurbedStreet c5{rect, start, stop};
            lanes.push_back({
                StreetWayPoint{.alpha{0.75f - shift, 0.25f + shift}, .edge{o23(c5.s00, c5.s01)}},
                StreetWayPoint{.alpha{0.75f - shift, 0.25f + shift}, .edge{o23(c5.s10, c5.s11)}}});
            lanes.push_back({
                StreetWayPoint{.alpha{0.25f + shift, 0.75f - shift}, .edge{o23(c5.s10, c5.s11)}},
                StreetWayPoint{.alpha{0.25f + shift, 0.75f - shift}, .edge{o23(c5.s00, c5.s01)}}});
        };
        add(-curb_alpha, curb_alpha, lane_shift, way_point_edge_descriptors[WayPointLocation::STREET]);
        if (curb2_alpha != 1) {
            add(curb_alpha, curb2_alpha, 0.f, way_point_edge_descriptors[WayPointLocation::SIDEWALK]);
            add(-curb2_alpha, -curb_alpha, 0.f, way_point_edge_descriptors[WayPointLocation::SIDEWALK]);
        }
    } else if (driving_direction == DrivingDirection::RIGHT) {
        auto add = [&rect](float start, float stop, float shift, std::list<std::pair<StreetWayPoint, StreetWayPoint>>& lanes){
            CurbedStreet c5{rect, start, stop};
            lanes.push_back({
                StreetWayPoint{.alpha{0.75f - shift, 0.25f + shift}, .edge{o23(c5.s10, c5.s11)}},
                StreetWayPoint{.alpha{0.75f - shift, 0.25f + shift}, .edge{o23(c5.s00, c5.s01)}}});
            lanes.push_back({
                StreetWayPoint{.alpha{0.25f + shift, 0.75f - shift}, .edge{o23(c5.s00, c5.s01)}},
                StreetWayPoint{.alpha{0.25f + shift, 0.75f - shift}, .edge{o23(c5.s10, c5.s11)}}});
        };
        add(-curb_alpha, curb_alpha, lane_shift, way_point_edge_descriptors[WayPointLocation::STREET]);
        if (curb2_alpha != 1) {
            add(curb_alpha, curb2_alpha, 0.f, way_point_edge_descriptors[WayPointLocation::SIDEWALK]);
            add(-curb2_alpha, -curb_alpha, 0.f, way_point_edge_descriptors[WayPointLocation::SIDEWALK]);
        }
    } else {
        throw std::runtime_error("Unknown driving direction");
    }
    {
        // Separate from waypoints, because the vertices must survive
        // application of the height-map.
        auto add = [this, &rect, &node_id, &angle_way](
            float start,
            float stop,
            WayPointLocation location,
            size_t nlanes,
            const std::string& bumps_model)
        {
            CurbedStreet c1{rect, start, stop};
            street_rectangles.push_back(StreetRectangle{
                .location = location,
                .road_properties = RoadProperties{
                    .type = angle_way.road_type,
                    .nlanes = nlanes
                },
                .bumps_model = bumps_model,
                .rectangle = FixedArray<FixedArray<double, 3>, 2, 2>{
                    FixedArray<double, 3>{c1.s00(0), c1.s00(1), 0.f},
                    FixedArray<double, 3>{c1.s01(0), c1.s01(1), 0.f},
                    FixedArray<double, 3>{c1.s10(0), c1.s10(1), 0.f},
                    FixedArray<double, 3>{c1.s11(0), c1.s11(1), 0.f}}});
        };
        std::string bumps_model = auto_model_name(
            node_id,
            angle_way,
            street_bumps_central_resource_names,
            street_bumps_endpoint0_resource_names,
            street_bumps_endpoint1_resource_names);
        add(-curb_alpha, curb_alpha, WayPointLocation::STREET, nlanes, bumps_model);
        if (curb2_alpha != 1) {
            add(-1, -curb2_alpha, WayPointLocation::SIDEWALK, 2, "");
            add(curb2_alpha, 1, WayPointLocation::SIDEWALK, 2, "");
        }
    }
}

std::string DrawStreets::auto_model_name(
    const std::string& node_id,
    const AngleWay& angle_way,
    const Map<RoadType, std::string>& central_resource_names,
    const Map<RoadType, std::string>& endpoint0_resource_names,
    const Map<RoadType, std::string>& endpoint1_resource_names) const
{
    auto model_name = [&](const std::map<RoadType, std::string>& res) -> std::optional<const std::string> {
        auto it = res.find(angle_way.road_type);
        if (it == res.end()) {
            return std::nullopt;
        } else {
            return it->second;
        }
    };
    std::optional<const std::string> model_name_central = model_name(central_resource_names);
    std::optional<const std::string> model_name_endpoint0 = model_name(endpoint0_resource_names);
    std::optional<const std::string> model_name_endpoint1 = model_name(endpoint1_resource_names);
    std::optional<const std::string> model_name_central_orig = model_name_central;
    auto get_neighbor_road_connection_type = [this](
        const std::string& node_id,
        const std::string& not_node_id,
        RoadType& rt,
        RoadConnectionType& rct)
    {
        const auto& node_angles0 = node_angles.at(node_id);
        if (node_angles0.size() != 2) {
            throw std::runtime_error("get_neighbor_road_connection_type internal error");
        }
        auto it0 = node_angles0.begin();
        if (it0->second.neighbor_id == not_node_id) {
            ++it0;
        }
        rt = it0->second.road_type;
        RoadConnectionType rct0;
        RoadConnectionType rct1;
        road_connection_types_from_model_name(way_infos.at(it0->second.way_id).model, rct0, rct1);
        if (it0->second.neighbor_is_second) {
            rct = rct0;
        } else {
            rct = rct1;
        }
    };
    const auto& node_angles0 = node_angles.at(node_id);
    const auto& node_angles1 = node_angles.at(angle_way.neighbor_id);
    // Way length is used to get connected street textures where possible.
    auto node_way_info0 = node_way_info.find(node_id);
    auto node_way_info1 = node_way_info.find(angle_way.neighbor_id);
    if (node_way_info0 == node_way_info.end()) {
        throw std::runtime_error("Could not find way info for node \"" + node_id + '"');
    }
    if (node_way_info1 == node_way_info.end()) {
        throw std::runtime_error("Could not find way info for node \"" + angle_way.neighbor_id + '"');
    }
    if (!central_resource_names.empty()) {
        if (node_angles0.size() != 2) {
            model_name_central = std::nullopt;
            model_name_endpoint0 = std::nullopt;
        } else {
            RoadType rt0;
            RoadConnectionType rct0;
            get_neighbor_road_connection_type(node_id, angle_way.neighbor_id, rt0, rct0);
            std::optional<const std::string> model_central_0 = central_resource_names.try_get(rt0);
            if ((node_way_info0 == node_way_info.end()) ||
                (rct0 == RoadConnectionType::ENDPOINT) ||
                std::isnan(node_way_info0->second.layer) ||
                (angle_way.layer != 0) ||
                (model_name_central_orig != model_central_0))
            {
                model_name_central = std::nullopt;
                model_name_endpoint0 = std::nullopt;
            }
        }
        if (node_angles1.size() != 2) {
            model_name_central = std::nullopt;
            model_name_endpoint1 = std::nullopt;
        } else {
            RoadType rt1;
            RoadConnectionType rct1;
            get_neighbor_road_connection_type(angle_way.neighbor_id, node_id, rt1, rct1);
            std::optional<const std::string> model_central_1 = central_resource_names.try_get(rt1);
            if ((node_way_info1 == node_way_info.end()) ||
                (rct1 == RoadConnectionType::ENDPOINT) ||
                std::isnan(node_way_info1->second.layer) ||
                (angle_way.layer != 0) ||
                (model_name_central_orig != model_central_1))
            {
                model_name_central = std::nullopt;
                model_name_endpoint1 = std::nullopt;
            }
        }
    }
    if ((model_name_central != std::nullopt) ||
        (model_name_endpoint0 != std::nullopt) ||
        (model_name_endpoint1 != std::nullopt))
    {
        if (model_name_central != std::nullopt) {
            assert_true(node_angles0.size() == 2);
            assert_true(node_angles1.size() == 2);
            if ((*model_name_central).empty()) {
                throw std::runtime_error("Empty model names not supported");
            }
            return *model_name_central;
        } else if (model_name_endpoint0 != std::nullopt) {
            // assert_true(node_angles.at(node_id).size() != 2);
            assert_true(node_angles0.size() == 2);
            if ((*model_name_endpoint0).empty()) {
                throw std::runtime_error("Empty model names not supported");
            }
            return *model_name_endpoint0;
        } else if (model_name_endpoint1 != std::nullopt) {
            assert_true(node_angles1.size() == 2);
            // assert_true(node_angles.at(angle_way.neighbor_id).size() != 2);
            if ((*model_name_endpoint1).empty()) {
                throw std::runtime_error("Empty model names not supported");
            }
            return *model_name_endpoint1;
        } else {
            throw std::runtime_error("Draw streets internal error");
        }
    }
    return "";
}

void DrawStreets::draw_streets_draw_ways(
    const OsmRectangle2D& rect,
    const std::string& node_id,
    const AngleWay& angle_way)
{
    auto sit = uv_scales.find(angle_way.road_type);
    if (sit == uv_scales.end()) {
        throw std::runtime_error("Could not find uv_scale for " + road_type_to_string(angle_way.road_type));
    }
    float uv_scale = sit->second;
    // Way length is used to get connected street textures where possible.
    auto node_way_info0 = node_way_info.find(node_id);
    auto node_way_info1 = node_way_info.find(angle_way.neighbor_id);
    if (node_way_info0 == node_way_info.end()) {
        throw std::runtime_error("Could not find way info for node \"" + node_id + '"');
    }
    if (node_way_info1 == node_way_info.end()) {
        throw std::runtime_error("Could not find way info for node \"" + angle_way.neighbor_id + '"');
    }
    const auto& node0 = nodes.at(node_id);
    const auto& node1 = nodes.at(angle_way.neighbor_id);
    const auto& node_angles0 = node_angles.at(node_id);
    const auto& node_angles1 = node_angles.at(angle_way.neighbor_id);
    auto& tlists = angle_way.layer == 0 ? ground_triangles : air_triangles;
    auto& street_lst = tlists.tl_street[RoadProperties{angle_way.road_type, angle_way.nlanes}];
    bool with_b_height_binding;
    bool with_c_height_binding;
    with_b_height_binding = with_height_bindings && !node0.tags.contains("bind_height", "no");
    with_c_height_binding = with_height_bindings && !node1.tags.contains("bind_height", "no");
    EntranceType b_entrance_type = EntranceType::NONE;
    EntranceType c_entrance_type = EntranceType::NONE;
    if (angle_way.layer < 0) {
        for (const auto& e : node_angles0) {
            if (e.second.layer == 0) {
                c_entrance_type = EntranceType::TUNNEL;
            }
        }
        for (const auto& e : node_angles1) {
            if (e.second.layer == 0) {
                b_entrance_type = EntranceType::TUNNEL;
            }
        }
    } else if (angle_way.layer > 0) {
        for (const auto& e : node_angles0) {
            if (e.second.layer == 0) {
                c_entrance_type = EntranceType::BRIDGE;
            }
        }
        for (const auto& e : node_angles1) {
            if (e.second.layer == 0) {
                b_entrance_type = EntranceType::BRIDGE;
            }
        }
    }
    if ((b_entrance_type != EntranceType::NONE) && (c_entrance_type != EntranceType::NONE)) {
        std::cerr << "WARNING: Detected two entrances at way " + node_id + " - " + angle_way.neighbor_id << std::endl;
        b_entrance_type = EntranceType::NONE;
        c_entrance_type = EntranceType::NONE;
    }
    EntranceType et = (b_entrance_type != EntranceType::NONE)
        ? b_entrance_type
        : c_entrance_type;
    const auto& wi = way_infos.at(angle_way.way_id);
    // Final u-coordinate: (u - d) * s + 0.5 = u*s - d*s + 0.5
    // where d = 0.5 * (beta + 1)
    double racing_line_beta0;
    double racing_line_beta1;
    bool flip_racing_line;
    const RacingLineSegment* racing_line_segment0;
    const RacingLineSegment* racing_line_segment1;
    float racing_line_segment_scale_x0;
    float racing_line_segment_scale_x1;
    {
        CurbedStreet c{rect, -wi.curb_alpha, wi.curb_alpha};
        racing_line_bvh.intersecting_way_beta({ c.s00, c.s01 }, racing_line_beta0, &racing_line_segment0);
        racing_line_bvh.intersecting_way_beta({ c.s10, c.s11 }, racing_line_beta1, &racing_line_segment1);
        racing_line_segment_scale_x0 = std::sqrt(sum(squared(c.s00 - c.s01))) / scale / racing_line_width_x / 2.f;
        racing_line_segment_scale_x1 = std::sqrt(sum(squared(c.s10 - c.s11))) / scale / racing_line_width_x / 2.f;
    }
    if (!std::isnan(racing_line_beta0) && !std::isnan(racing_line_beta1)) {
        auto v0 = racing_line_segment0->racing_line_segment(1) - racing_line_segment0->racing_line_segment(0);
        auto v1 = racing_line_segment1->racing_line_segment(1) - racing_line_segment1->racing_line_segment(0);
        auto vs = node1.position - node0.position;
        if ((dot0d(v0, vs) > 0) && (dot0d(v1, vs) > 0)) {
            flip_racing_line = false;
        } else if ((dot0d(v0, vs) < 0) && (dot0d(v1, vs) < 0)) {
            flip_racing_line = true;
        } else {
            std::cerr << "WARNING: detected inconsistent racing line direction" << std::endl;
            racing_line_beta0 = NAN;
            racing_line_beta1 = NAN;
        }
    } else {
        racing_line_beta0 = NAN;
        racing_line_beta1 = NAN;
    }
    float racing_line_d0 = 0.5f * (racing_line_beta0 + 1.f);
    float racing_line_d1 = 0.5f * (racing_line_beta1 + 1.f);
    float racing_line_dx0 = -racing_line_segment_scale_x0 * racing_line_d0 + 0.5f;
    float racing_line_dx1 = -racing_line_segment_scale_x1 * racing_line_d1 + 0.5f;
    float uv_len0;
    float uv_len1;
    if (!std::isnan(node_way_info0->second.way_length) &&
        !std::isnan(node_way_info1->second.way_length))
    {
        uv_len0 = node_way_info0->second.way_length / scale * uv_scale;
        uv_len1 = node_way_info1->second.way_length / scale * uv_scale;
    } else {
        uv_len0 = 0;
        uv_len1 = std::sqrt(sum(squared(node0.position - node1.position))) / scale * uv_scale;
    }
    if ((street_surface_central_resource_names.empty() != street_surface_endpoint0_resource_names.empty()) ||
        (street_surface_central_resource_names.empty() != street_surface_endpoint1_resource_names.empty())) {
        throw std::runtime_error("Inconsistent definition of surface central / endpoint");
    }
    auto draw_street_with_ditch = [&](
        const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
        const std::string& model_name)
    {
        for (const auto& cva : cvas) {
            TriangleList<double>* destination_triangles;
            if (cva->name == "street") {
                destination_triangles = street_lst.triangle_list.get();
            } else if (cva->name == "curb") {
                destination_triangles = tlists.tl_street_curb[angle_way.road_type].get();
            } else if (cva->name == "ditch") {
                destination_triangles = tlists.tl_ditch.get();
            } else {
                throw std::runtime_error("Unknown street name \"" + cva->name + "\", must be \"street\" or \"ditch\"");
            }
            assert_true(angle_way.neighbor_is_second);
            try {
                rect.draw(
                    *destination_triangles,
                    !std::isnan(racing_line_dx0) && (cva->name == "street")
                        ? tlists.tl_racing_line.get()
                        : nullptr,
                    racing_line_segment_scale_x0,
                    racing_line_segment_scale_x1,
                    racing_line_dx0,
                    racing_line_dx1,
                    flip_racing_line,
                    !std::isnan(racing_line_dx0) ? racing_line_segment0->color : fixed_nans<float, 3>(),
                    !std::isnan(racing_line_dx1) ? racing_line_segment1->color : fixed_nans<float, 3>(),
                    node_height_bindings,
                    node_id,
                    angle_way.neighbor_id,
                    cva->triangles,
                    scale,
                    wi.curb_alpha,
                    1.f,
                    uv_len0,
                    uv_len1);
            } catch (const std::runtime_error& e) {
                throw std::runtime_error("Could not draw street model \"" + model_name + "\": " + e.what());
            }
        }
    };
    auto draw_procedural_street = [&](){
        rect.draw_z0(
            *street_lst.triangle_list,
            std::isnan(racing_line_dx0)
                ? nullptr
                : tlists.tl_racing_line.get(),
            racing_line_segment_scale_x0,
            racing_line_segment_scale_x1,
            racing_line_dx0,
            racing_line_dx1,
            flip_racing_line,
            !std::isnan(racing_line_dx0) ? racing_line_segment0->color : fixed_nans<float, 3>(),
            !std::isnan(racing_line_dx1) ? racing_line_segment1->color : fixed_nans<float, 3>(),
            ground_triangles.tl_entrance[et].get(),
            node_height_bindings,
            ground_triangles.entrances,
            node_id,
            angle_way.neighbor_id,
            wi.colors(0),
            wi.colors(0),
            0,
            street_lst.uvx,
            uv_len0,
            uv_len1,
            -wi.curb_alpha,
            wi.curb_alpha,
            RectangleOrientation::CENTER,
            with_b_height_binding,
            with_c_height_binding,
            b_entrance_type,
            c_entrance_type,
            angle_way.road_type);
    };
    if (wi.model.empty()) {
        std::string model_name = auto_model_name(
            node_id,
            angle_way,
            street_surface_central_resource_names,
            street_surface_endpoint0_resource_names,
            street_surface_endpoint1_resource_names);
        if (!model_name.empty()) {
            draw_street_with_ditch(scene_node_resources.get_animated_arrays(model_name)->scvas, model_name);
        } else {
            draw_procedural_street();
        }
    } else if (wi.model == "endpoint") {
        draw_procedural_street();
    } else {
        draw_street_with_ditch(scene_node_resources.get_animated_arrays(wi.model)->scvas, wi.model);
    }
    if (angle_way.layer > 0) {
        rect.draw_z0(
            *air_triangles.tl_air_support,
            nullptr,
            NAN,
            NAN,
            NAN,
            NAN,
            false,
            fixed_nans<float, 3>(),
            fixed_nans<float, 3>(),
            nullptr,
            node_height_bindings,
            ground_triangles.entrances,
            node_id,
            angle_way.neighbor_id,
            wi.colors(0),
            wi.colors(0),
            0,
            1,
            uv_len0,
            uv_len1,
            -1,
            1,
            RectangleOrientation::CENTER,
            with_b_height_binding,
            with_c_height_binding,
            b_entrance_type,
            c_entrance_type,
            angle_way.road_type);
    }
    if (angle_way.layer < 0) {
        auto draw = [&](auto& lst, auto& mesh){rect.draw(
            lst,
            nullptr,
            NAN,
            NAN,
            NAN,
            NAN,
            false,
            fixed_nans<float, 3>(),
            fixed_nans<float, 3>(),
            node_height_bindings,
            node_id,
            angle_way.neighbor_id,
            mesh, scale,
            default_tunnel_pipe_width,
            default_tunnel_pipe_height,
            NAN,
            NAN);};
        draw(*air_triangles.tl_tunnel_pipe, tunnel_pipe_triangles);
        draw(*air_triangles.tl_tunnel_bdry, tunnel_bdry_triangles);
    }
    if ((wi.curb_alpha != 1) && !wi.roads_delete(1)) {
        if (!wi.roads_delete_side(angle_way.neighbor_is_second)(1)) {
            rect.draw_z0(
                *tlists.tl_street_curb[angle_way.road_type],
                nullptr,
                NAN,
                NAN,
                NAN,
                NAN,
                false,
                fixed_nans<float, 3>(),
                fixed_nans<float, 3>(),
                ground_triangles.tl_entrance[et].get(),
                node_height_bindings,
                ground_triangles.entrances,
                node_id,
                angle_way.neighbor_id,
                wi.colors(1),
                wi.colors(1),
                0,
                curb_uv(0),
                curb_uv(1) * uv_len0,
                curb_uv(1) * uv_len1,
                -wi.curb2_alpha,
                -wi.curb_alpha,
                RectangleOrientation::RIGHT,
                with_b_height_binding,
                with_c_height_binding,
                b_entrance_type,
                c_entrance_type,
                angle_way.road_type);
        }
        if (!wi.roads_delete_side(!angle_way.neighbor_is_second)(1)) {
            rect.draw_z0(
                *tlists.tl_street_curb[angle_way.road_type],
                nullptr,
                NAN,
                NAN,
                NAN,
                NAN,
                false,
                fixed_nans<float, 3>(),
                fixed_nans<float, 3>(),
                ground_triangles.tl_entrance[et].get(),
                node_height_bindings,
                ground_triangles.entrances,
                node_id,
                angle_way.neighbor_id,
                wi.colors(1),
                wi.colors(1),
                0,
                curb_uv(0),
                curb_uv(1) * uv_len0,
                curb_uv(1) * uv_len1,
                wi.curb_alpha,
                wi.curb2_alpha,
                RectangleOrientation::LEFT,
                with_b_height_binding,
                with_c_height_binding,
                b_entrance_type,
                c_entrance_type,
                angle_way.road_type);
        }
    }
    if ((wi.curb2_alpha != 1) && !wi.roads_delete(2)) {
        if (!wi.roads_delete_side(angle_way.neighbor_is_second)(2)) {
            rect.draw_z0(
                *tlists.tl_street_curb2[angle_way.road_type],
                nullptr,
                NAN,
                NAN,
                NAN,
                NAN,
                false,
                fixed_nans<float, 3>(),
                fixed_nans<float, 3>(),
                ground_triangles.tl_entrance[et].get(),
                node_height_bindings,
                ground_triangles.entrances,
                node_id,
                angle_way.neighbor_id,
                wi.colors(2),
                wi.colors(2),
                0,
                curb2_uv(0),
                curb2_uv(1) * uv_len0,
                curb2_uv(1) * uv_len1,
                -1,
                -wi.curb2_alpha,
                RectangleOrientation::RIGHT,
                with_b_height_binding,
                with_c_height_binding,
                b_entrance_type,
                c_entrance_type,
                angle_way.road_type);
        }
        if (!wi.roads_delete_side(!angle_way.neighbor_is_second)(2)) {
            rect.draw_z0(
                *tlists.tl_street_curb2[angle_way.road_type],
                nullptr,
                NAN,
                NAN,
                NAN,
                NAN,
                false,
                fixed_nans<float, 3>(),
                fixed_nans<float, 3>(),
                ground_triangles.tl_entrance[et].get(),
                node_height_bindings,
                ground_triangles.entrances,
                node_id,
                angle_way.neighbor_id,
                wi.colors(2),
                wi.colors(2),
                0,
                curb2_uv(0),
                curb2_uv(1) * uv_len0,
                curb2_uv(1) * uv_len1,
                wi.curb2_alpha,
                1,
                RectangleOrientation::LEFT,
                with_b_height_binding,
                with_c_height_binding,
                b_entrance_type,
                c_entrance_type,
                angle_way.road_type);
        }
    }
}

void DrawStreets::draw_streets_find_hole_contours(
    const OsmRectangle2D& rect,
    const std::string& node_id,
    const AngleWay& angle_way,
    float node_angle)
{
    auto& air_hole_list = (angle_way.layer > 0)
        ? air_support_node_hole_contours
        : tunnel_node_hole_contours;
    const std::map<std::string, NeighborWay>& na = node_neighbors.at(node_id);
    const auto& wi = way_infos.at(angle_way.way_id);
    if (na.size() >= 3) {
        {
            CurbedStreet c0{rect, -wi.curb_alpha, wi.curb_alpha};
            node_hole_contours.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = 0}, NodeHoleVertex{c0.s00, angle_way.way_id}));
        }
        if (angle_way.layer != 0) {
            air_hole_list.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = 0}, NodeHoleVertex{rect.p00_, angle_way.way_id}));
        }
        if (wi.curb_alpha != 1) {
            CurbedStreet cN{rect, -wi.curb2_alpha, -wi.curb_alpha};
            CurbedStreet cP{rect, wi.curb_alpha, wi.curb2_alpha};
            node_hole_contours.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = +1}, NodeHoleVertex{cN.s00, angle_way.way_id}));
            node_hole_contours.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = -1}, NodeHoleVertex{cP.s00, angle_way.way_id}));
        }
        if (wi.curb2_alpha != 1) {
            CurbedStreet cN{rect, -1, -wi.curb2_alpha};
            CurbedStreet cP{rect, wi.curb2_alpha, 1};
            node_hole_contours.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = +2}, NodeHoleVertex{cN.s00, angle_way.way_id}));
            node_hole_contours.at(node_id).insert(std::make_pair(AngleCurb{.angle = node_angle, .curb = -2}, NodeHoleVertex{cP.s00, angle_way.way_id}));
        }
    }
    const std::map<std::string, NeighborWay>& nn = node_neighbors.at(angle_way.neighbor_id);
    if (nn.size() >= 3) {
        {
            CurbedStreet c0{rect, -wi.curb_alpha, wi.curb_alpha};
            // Left and right are swapped for the neighbor, so we use p11_ instead of p10_.
            node_hole_contours.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = 0}, NodeHoleVertex{c0.s11, angle_way.way_id}));
        }
        if (angle_way.layer != 0) {
            air_hole_list.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = 0}, NodeHoleVertex{rect.p11_, angle_way.way_id}));
        }
        if (wi.curb_alpha != 1) {
            CurbedStreet cN{rect, -wi.curb2_alpha, -wi.curb_alpha};
            CurbedStreet cP{rect, wi.curb_alpha, wi.curb2_alpha};
            node_hole_contours.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = -1}, NodeHoleVertex{cN.s11, angle_way.way_id}));
            node_hole_contours.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = +1}, NodeHoleVertex{cP.s11, angle_way.way_id}));
        }
        if (wi.curb2_alpha != 1) {
            CurbedStreet cN{rect, -1, -wi.curb2_alpha};
            CurbedStreet cP{rect, wi.curb2_alpha, 1};
            node_hole_contours.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = -2}, NodeHoleVertex{cN.s11, angle_way.way_id}));
            node_hole_contours.at(angle_way.neighbor_id).insert(std::make_pair(AngleCurb{.angle = nn.at(node_id).angle, .curb = +2}, NodeHoleVertex{cP.s11, angle_way.way_id}));
        }
    }
}

void DrawStreets::draw_streets_find_hole_waypoints(
    const OsmRectangle2D& rect,
    const std::string& node_id,
    const std::string& neighbor_id,
    float curb_alpha,
    float curb2_alpha,
    float lane_shift)
{
    const std::map<std::string, NeighborWay>& na = node_neighbors.at(node_id);
    if (na.size() >= 3) {
        if (driving_direction == DrivingDirection::LEFT) {
            auto add = [&rect, &node_id, &neighbor_id](float start, float stop, float shift, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                CurbedStreet c5{rect, start, stop};
                node_hole_waypoints.at(node_id).out.push_back(NodeHoleWaypoint{.node=neighbor_id, .alpha{0.75f - shift, 0.25f + shift}, .edge{c5.s00, c5.s01}});
                node_hole_waypoints.at(node_id).in.push_back(NodeHoleWaypoint{.node=neighbor_id, .alpha{0.25f + shift, 0.75f - shift}, .edge{c5.s00, c5.s01}});
            };
            add(-curb_alpha, curb_alpha, lane_shift, node_hole_waypoints_street);
            if (curb2_alpha != 1) {
                add(curb_alpha, curb2_alpha, 0.f, node_hole_waypoints_sidewalk);
                add(-curb2_alpha, -curb_alpha, 0.f, node_hole_waypoints_sidewalk);
            }
        } else if (driving_direction == DrivingDirection::RIGHT) {
            auto add = [&rect, &node_id, &neighbor_id](float start, float stop, float shift, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                CurbedStreet c5{rect, start, stop};
                node_hole_waypoints.at(node_id).in.push_back(NodeHoleWaypoint{.node=neighbor_id, .alpha{0.75f - shift, 0.25f + shift}, .edge{c5.s00, c5.s01}});
                node_hole_waypoints.at(node_id).out.push_back(NodeHoleWaypoint{.node=neighbor_id, .alpha{0.25f + shift, 0.75f - shift}, .edge{c5.s00, c5.s01}});
            };
            add(-curb_alpha, curb_alpha, lane_shift, node_hole_waypoints_street);
            if (curb2_alpha != 1) {
                add(curb_alpha, curb2_alpha, 0.f, node_hole_waypoints_sidewalk);
                add(-curb2_alpha, -curb_alpha, 0.f, node_hole_waypoints_sidewalk);
            }
        } else if (driving_direction != DrivingDirection::CENTER) {
            throw std::runtime_error("Unknown driving direction");
        }
    }
    const std::map<std::string, NeighborWay>& nn = node_neighbors.at(neighbor_id);
    if (nn.size() >= 3) {
        if (driving_direction == DrivingDirection::LEFT) {
            auto add = [&rect, &neighbor_id, &node_id](float start, float stop, float shift, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                CurbedStreet c5{rect, start, stop};
                node_hole_waypoints.at(neighbor_id).out.push_back(NodeHoleWaypoint{.node=node_id, .alpha{0.25f + shift, 0.75f - shift}, .edge{c5.s10, c5.s11}});
                node_hole_waypoints.at(neighbor_id).in.push_back(NodeHoleWaypoint{.node=node_id, .alpha{0.75f - shift, 0.25f + shift}, .edge{c5.s10, c5.s11}});
            };
            add(-curb_alpha, curb_alpha, lane_shift, node_hole_waypoints_street);
            if (curb2_alpha != 1) {
                add(curb_alpha, curb2_alpha, 0.f, node_hole_waypoints_sidewalk);
                add(-curb2_alpha, -curb_alpha, 0.f, node_hole_waypoints_sidewalk);
            }
        } else if (driving_direction == DrivingDirection::RIGHT) {
            auto add = [&rect, &neighbor_id, &node_id](float start, float stop, float shift, std::map<std::string, HoleWaypoint>& node_hole_waypoints){
                CurbedStreet c5{rect, start, stop};
                node_hole_waypoints.at(neighbor_id).in.push_back(NodeHoleWaypoint{.node=node_id, .alpha{0.25f + shift, 0.75f - shift}, .edge{c5.s10, c5.s11}});
                node_hole_waypoints.at(neighbor_id).out.push_back(NodeHoleWaypoint{.node=node_id, .alpha{0.75f - shift, 0.25f + shift}, .edge{c5.s10, c5.s11}});
            };
            add(-curb_alpha, curb_alpha, lane_shift, node_hole_waypoints_street);
            if (curb2_alpha != 1) {
                add(curb_alpha, curb2_alpha, 0.f, node_hole_waypoints_sidewalk);
                add(-curb2_alpha, -curb_alpha, 0.f, node_hole_waypoints_sidewalk);
            }
        } else if (driving_direction != DrivingDirection::CENTER) {
            throw std::runtime_error("Unknown driving direction");
        }
    }
}
