#include "Parse_Osm_Xml.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Geography/Geographic_Coordinates.hpp>
#include <Mlib/Geometry/Coordinates/Normalized_Points_Fixed.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Strings/To_Number.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>
#include <iomanip>
#include <iostream>

using namespace Mlib;

void Mlib::parse_osm_xml(
    const std::string& filename,
    double scale,
    NormalizedPointsFixed<double>& normalized_points,
    std::optional<TransformationMatrix<double, double, 2>>& normalization_matrix,
    std::map<std::string, Node>& nodes,
    std::map<std::string, Way>& ways)
{
    auto ifs_p = create_ifstream(filename);
    auto& ifs = *ifs_p;
    if (ifs.fail()) {
        THROW_OR_ABORT("Could not open OSM XML-file \"" + filename + '"');
    }
    static const DECLARE_REGEX(node_reg, "^ +<node id=[\"'](-?\\w+)[\"'](?: action=[\"']([^\"']+)[\"'])? .*visible=[\"'](true|false)[\"'].* lat=[\"']([\\w.-]+)[\"'] lon=[\"']([\\w.-]+)[\"'].*>$");
    static const DECLARE_REGEX(node_end_reg, " +</node>");
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
        " +<relation .*|"
        "  +<member .*|"
        " +</relation>|"
        "</osm>)$");

    FixedArray<double, 2> bounds_min_merged = fixed_full<double, 2>(INFINITY);
    FixedArray<double, 2> bounds_max_merged = fixed_full<double, 2>(-INFINITY);
    FixedArray<double, 2> current_node_position = fixed_nans<double, 2>();
    std::string current_way = "<none>";
    std::string current_node = "<none>";
    std::map<OrderableFixedArray<CompressedScenePos, 2>, std::string> ordered_node_positions;

    std::string line;
    while(std::getline(ifs, line)) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0) {
            continue;
        }
        Mlib::re::cmatch match;
        if (Mlib::re::regex_match(line, ignored_reg)) {
            // do nothing
        } else if (Mlib::re::regex_match(line, match, bounds_reg)) {
            if (!nodes.empty()) {
                THROW_OR_ABORT("Found bounds section, but nodes were already computed");
            }
            FixedArray<double, 2> bounds_min{
                safe_stod(match[1].str()),
                safe_stod(match[2].str())};
            FixedArray<double, 2> bounds_max{
                safe_stod(match[3].str()),
                safe_stod(match[4].str())};
            bounds_min_merged = minimum(bounds_min, bounds_min_merged);
            bounds_max_merged = maximum(bounds_max, bounds_max_merged);
            if (normalization_matrix.has_value()) {
                linfo() << "merged bounds";
                linfo() << "min lat " << std::setprecision(18) << bounds_min_merged(0);
                linfo() << "min lon " << std::setprecision(18) << bounds_min_merged(1);
                linfo() << "max lat " << std::setprecision(18) << bounds_max_merged(0);
                linfo() << "max lon " << std::setprecision(18) << bounds_max_merged(1);
            }
            auto coords_ref = (bounds_min_merged + bounds_max_merged) / 2.0;
            auto m = latitude_longitude_2_meters_mapping(
                coords_ref(0),
                coords_ref(1)).pre_scaled(scale);
            FixedArray<double, 2> min = m.transform(bounds_min_merged);
            FixedArray<double, 2> max = m.transform(bounds_max_merged);
            // Scale converts from meters to e.g. kilometers
            normalized_points.set_min(min);
            normalized_points.set_max(max);
            normalization_matrix = normalized_points.normalization_matrix() * m;
        } else if (Mlib::re::regex_match(line, match, node_reg)) {
            current_way = "<none>";
            std::string action = match[2].str();
            std::string visible = match[3].str();
            if (!normalization_matrix.has_value()) {
                THROW_OR_ABORT("Normalization-matrix undefined, bounds-section?");
            }
            if ((action != "delete") && (visible == "true")) {
                current_node = match[1].str();
                std::string lat = match[4].str();
                std::string lon = match[5].str();
                if (nodes.find(current_node) != nodes.end()) {
                    THROW_OR_ABORT("Found duplicate node id: " + current_node);
                }
                current_node_position = FixedArray<double, 2>{
                    safe_stod(lat),
                    safe_stod(lon)};
                auto pos = normalization_matrix->transform(current_node_position).casted<CompressedScenePos>();
                auto opos = OrderableFixedArray<CompressedScenePos, 2>{ pos };
                auto it = ordered_node_positions.find(opos);
                if (it != ordered_node_positions.end()) {
                    lwarn() << "Detected duplicate points: " + current_node + ", " + it->second;
                } else {
                    ordered_node_positions.insert(std::make_pair(opos, current_node));
                }
                nodes.insert(std::make_pair(current_node, Node{.position = pos}));
                // float dist = sum(squared(pos - FixedArray<float, 2>{-0.801262, 0.0782831}));
                // if (dist < 1e-3) {
                //     lerr() << "err: " << dist << " " << match[1].str();
                // }
            } else {
                current_node = "<none>";
            }
        } else if (Mlib::re::regex_match(line, match, node_end_reg)) {
            if ((current_node != "<none>") && !nodes.at(current_node).tags.contains("height_reference", "water")) {
                if (any(isnan(current_node_position))) {
                    THROW_OR_ABORT("Closing node tag with NAN position");
                }
                if (any(current_node_position < bounds_min_merged - FixedArray<double, 2>{0.01, 0.01})) {
                    std::stringstream sstr;
                    sstr << "Node with ID " << current_node << " and coordinates " << current_node_position << " is out of minimum bounds " << bounds_min_merged;
                    THROW_OR_ABORT(sstr.str());
                }
                if (any(current_node_position > bounds_max_merged + FixedArray<double, 2>{0.01, 0.01})) {
                    std::stringstream sstr;
                    sstr << "Node with ID " << current_node << " and coordinates " << current_node_position << " is out of maximum bounds " << bounds_max_merged;
                    THROW_OR_ABORT(sstr.str());
                }
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
                THROW_OR_ABORT("No current way");
            }
            if (current_way != "<invisible>") {
                auto it = ways.find(current_way);
                if (it == ways.end()) {
                    THROW_OR_ABORT("Could not find way with ID " + current_way);
                }
                it->second.nd.push_back(match[1].str());
            }
        } else  if (Mlib::re::regex_match(line, match, tag_reg)) {
            assert_true((current_node == "<none>") || (current_way == "<none>"));
            auto tag = std::make_pair(match[1].str(), match[2].str());
            if (current_node != "<none>") {
                auto it = nodes.find(current_node);
                if (it == nodes.end()) {
                    THROW_OR_ABORT("Could not find node with ID " + current_node);
                }
                if (!it->second.tags.insert(tag).second) {
                    THROW_OR_ABORT("Duplicate node tag " + tag.first + " for node with ID " + current_node);
                }
            }
            if (current_way != "<none>") {
                if (current_way != "<invisible>") {
                    auto it = ways.find(current_way);
                    if (it == ways.end()) {
                        THROW_OR_ABORT("Could not find way with ID " + current_way);
                    }
                    if (!it->second.tags.insert(tag).second) {
                        THROW_OR_ABORT("Duplicate way tag " + tag.first);
                    }
                }
            }
        } else if (Mlib::re::regex_match(line, way_end_reg)) {
            current_way = "<none>";
        } else {
            THROW_OR_ABORT("Could not parse line " + line);
        }
    }

    if (!ifs.eof() && ifs.fail()) {
        THROW_OR_ABORT("Parse OSM XML: Error reading from file \"" + filename + '"');
    }
}
