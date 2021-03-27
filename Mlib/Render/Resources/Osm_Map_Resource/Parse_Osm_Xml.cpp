#include "Parse_Osm_Xml.hpp"
#include <Mlib/Geometry/Normalized_Points_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Geographic_Coordinates.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace Mlib;

void Mlib::parse_osm_xml(
    const std::string& filename,
    float scale,
    NormalizedPointsFixed& normalized_points,
    TransformationMatrix<double, 2>& normalization_matrix,
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways)
{
    std::ifstream ifs{ filename };
    static const DECLARE_REGEX(node_reg, "^ +<node id=[\"'](-?\\w+)[\"'](?: action=[\"']([^\"']+)[\"'])? .*visible=[\"'](true|false)[\"'].* lat=[\"']([\\w.-]+)[\"'] lon=[\"']([\\w.-]+)[\"'].*>$");
    static const DECLARE_REGEX(way_reg, "^ +<way id=[\"'](-?\\w+)[\"'](?: action=[\"']([^\"']+)[\"'])? .*visible=[\"'](true|false)[\"'].*>$");
    static const DECLARE_REGEX(way_end_reg, "^ +</way>$");
    static const DECLARE_REGEX(node_ref_reg, "^  +<nd ref=[\"'](-?\\w+)[\"'] */>$");
    static const DECLARE_REGEX(bounds_reg, " +<bounds minlat=[\"']([\\w.-]+)[\"'] minlon=[\"']([\\w.-]+)[\"'] maxlat=[\"']([\\w.-]+)[\"'] maxlon=[\"']([\\w.-]+)[\"'](?: origin=[\"'].*[\"'])? */>$");
    static const DECLARE_REGEX(tag_reg, "  +<tag k=[\"']([\\w:;.&|*=-]+)[\"'] v=[\"'](.*)[\"'] */>$");
    static const DECLARE_REGEX(ignored_reg,
        "^(?:"
        "<\\?xml .*|"
        "<osm .*|"
        " +<node.*action=[\"']delete[\"'].*/>|"
        " +</node>|"
        " +<relation .*|"
        "  +<member .*|"
        " +</relation>|"
        "</osm>)$");

    FixedArray<double, 2> bounds_min_merged = fixed_full<double, 2>(INFINITY);
    FixedArray<double, 2> bounds_max_merged = fixed_full<double, 2>(-INFINITY);
    FixedArray<double, 2> coords_ref;
    bool normalization_matrix_defined = false;
    std::string current_way = "<none>";
    std::string current_node = "<none>";
    std::map<OrderableFixedArray<float, 2>, std::string> ordered_node_positions;

    std::string line;
    while(std::getline(ifs, line)) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0) {
            continue;
        }
        Mlib::re::smatch match;
        if (Mlib::re::regex_match(line, ignored_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, bounds_reg)) {
            FixedArray<double, 2> bounds_min{
                safe_stod(match[1].str()),
                safe_stod(match[2].str())};
            FixedArray<double, 2> bounds_max{
                safe_stod(match[3].str()),
                safe_stod(match[4].str())};
            bounds_min_merged = minimum(bounds_min, bounds_min_merged);
            bounds_max_merged = maximum(bounds_max, bounds_max_merged);
            coords_ref = (bounds_min_merged + bounds_max_merged) / 2.0;
            auto m = latitude_longitude_2_meters_mapping(
                coords_ref(0),
                coords_ref(1)).pre_scaled(scale);
            FixedArray<float, 2> min = m.transform(bounds_min_merged).casted<float>();
            FixedArray<float, 2> max = m.transform(bounds_max_merged).casted<float>();
            // Scale converts from meters to e.g. kilometers
            normalized_points.set_min(min);
            normalized_points.set_max(max);
            normalization_matrix = normalized_points.normalization_matrix().casted<double>() * m;
            if (normalization_matrix_defined) {
                std::cerr << "merged bounds" << std::endl;
                std::cerr << "min lat " << std::setprecision(15) << bounds_min_merged(0) << std::endl;
                std::cerr << "min lon " << std::setprecision(15) << bounds_min_merged(1) << std::endl;
                std::cerr << "max lat " << std::setprecision(15) << bounds_max_merged(0) << std::endl;
                std::cerr << "max lon " << std::setprecision(15) << bounds_max_merged(1) << std::endl;
            }
            normalization_matrix_defined = true;
        } else if (Mlib::re::regex_match(line, match, node_reg)) {
            current_way = "<none>";
            std::string action = match[2].str();
            std::string visible = match[3].str();
            if (!normalization_matrix_defined) {
                throw std::runtime_error("Normalization-matrix undefined, bounds-section?");
            }
            if ((action != "delete") && (visible == "true")) {
                current_node = match[1].str();
                std::string lat = match[4].str();
                std::string lon = match[5].str();
                if (nodes.find(current_node) != nodes.end()) {
                    throw std::runtime_error("Found duplicate node id: " + current_node);
                }
                auto rpos = FixedArray<double, 2>{
                    safe_stod(lat),
                    safe_stod(lon)};
                if (any(rpos < bounds_min_merged - FixedArray<double, 2>{0.01, 0.01})) {
                    std::stringstream sstr;
                    sstr << "Node with ID " << current_node << " and coordinates " << rpos << " is out of minimum bounds " << bounds_min_merged << std::endl;
                    throw std::runtime_error(sstr.str());
                }
                if (any(rpos > bounds_max_merged + FixedArray<double, 2>{0.01, 0.01})) {
                    std::stringstream sstr;
                    sstr << "Node with ID " << current_node << " and coordinates " << rpos << " is out of maximum bounds " << bounds_max_merged << std::endl;
                    throw std::runtime_error(sstr.str());
                }
                auto pos = normalization_matrix.transform(rpos).casted<float>();
                auto opos = OrderableFixedArray<float, 2>{pos};
                auto it = ordered_node_positions.find(opos);
                if (it != ordered_node_positions.end()) {
                    std::cerr << "Detected duplicate points: " + current_node + ", " + it->second << std::endl;
                } else {
                    ordered_node_positions.insert(std::make_pair(opos, current_node));
                }
                nodes.insert(std::make_pair(current_node, Node{.position = pos}));
                // float dist = sum(squared(pos - FixedArray<float, 2>{-0.801262, 0.0782831}));
                // if (dist < 1e-3) {
                //     std::cerr << "err: " << dist << " " << match[1].str() << std::endl;
                // }
            } else {
                current_node = "<none>";
            }
        } else if (Mlib::re::regex_match(line, match, way_reg)) {
            current_node = "<none>";
            std::string action = match[2].str();
            std::string visible = match[3].str();
            if ((action != "delete") && (visible == "true")) {
                std::string way_id = match[1].str();
                current_way = way_id;
                ways.insert(std::make_pair(current_way, Way()));
            } else {
                current_way = "<invisible>";
            }
        } else if (Mlib::re::regex_match(line, match, node_ref_reg)) {
            if (current_way == "<none>") {
                throw std::runtime_error("No current way");
            }
            if (current_way != "<invisible>") {
                auto it = ways.find(current_way);
                if (it == ways.end()) {
                    throw std::runtime_error("Could not find way with ID " + current_way);
                }
                it->second.nd.push_back(match[1].str());
            }
        } else  if (Mlib::re::regex_match(line, match, tag_reg)) {
            assert_true((current_node == "<none>") || (current_way == "<none>"));
            auto tag = std::make_pair(match[1].str(), match[2].str());
            if (current_node != "<none>") {
                auto it = nodes.find(current_node);
                if (it == nodes.end()) {
                    throw std::runtime_error("Could not find node with ID " + current_node);
                }
                if (!it->second.tags.insert(tag).second) {
                    throw std::runtime_error("Duplicate node tag " + tag.first + " for node with ID " + current_node);
                }
            }
            if (current_way != "<none>") {
                if (current_way != "<invisible>") {
                    auto it = ways.find(current_way);
                    if (it == ways.end()) {
                        throw std::runtime_error("Could not find way with ID " + current_way);
                    }
                    if (!it->second.tags.insert(tag).second) {
                        throw std::runtime_error("Duplicate way tag " + tag.first);
                    }
                }
            }
        } else if (Mlib::re::regex_match(line, way_end_reg)) {
            current_way = "<none>";
        } else {
            throw std::runtime_error("Could not parse line " + line);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        throw std::runtime_error("Error reading from file \"" + filename + '"');
    }
}
