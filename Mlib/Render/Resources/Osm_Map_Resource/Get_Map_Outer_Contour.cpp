#include "Get_Map_Outer_Contour.hpp"
#include <Mlib/Render/Resources/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <stdexcept>

using namespace Mlib;

std::vector<FixedArray<float, 2>> Mlib::get_map_outer_contour(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    std::vector<FixedArray<float, 2>> contour;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.find("name") != tags.end() && tags.at("name") == "map-outer-contour") {
            if (!contour.empty()) {
                throw std::runtime_error("Found multiple map contours");
            }
            contour.reserve(w.second.nd.size());
            if (w.second.nd.empty()) {
                throw std::runtime_error("Map outer contour is empty");
            }
            for (auto it = w.second.nd.begin(); ; ++it) {
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
    return contour;
}
