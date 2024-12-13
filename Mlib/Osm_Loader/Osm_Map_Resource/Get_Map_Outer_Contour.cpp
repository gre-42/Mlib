#include "Get_Map_Outer_Contour.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

UUVector<FixedArray<CompressedScenePos, 2>> Mlib::get_map_outer_contour(
    const std::map<std::string, Node>& nodes,
    const std::map<std::string, Way>& ways)
{
    UUVector<FixedArray<CompressedScenePos, 2>> contour;
    for (const auto& w : ways) {
        const auto& tags = w.second.tags;
        if (tags.contains("name", "map-outer-contour")) {
            if (!contour.empty()) {
                THROW_OR_ABORT("Found multiple map contours");
            }
            contour.reserve(w.second.nd.size());
            if (w.second.nd.empty()) {
                THROW_OR_ABORT("Map outer contour is empty");
            }
            for (auto it = w.second.nd.begin(); ; ++it) {
                auto s = it;
                ++s;
                if (s == w.second.nd.end()) {
                    if (*it != *w.second.nd.begin()) {
                        THROW_OR_ABORT("Map outer contour not closed");
                    }
                    break;
                }
                contour.push_back(nodes.at(*it).position);
           }
        }
    }
    return contour;
}
