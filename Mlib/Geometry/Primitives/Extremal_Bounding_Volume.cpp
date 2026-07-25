#include "Extremal_Bounding_Volume.hpp"
#include <map>
#include <stdexcept>

using namespace Mlib;

ExtremalBoundingVolume Mlib::extremal_bounding_volume_from_string(const std::string& s) {
    static const std::map<std::string, ExtremalBoundingVolume> m{
        {"empty", ExtremalBoundingVolume::EMPTY},
        {"full", ExtremalBoundingVolume::FULL}
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown extremal bounding volume: \"" + s + '"');
    }
    return it->second;
}
