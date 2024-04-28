#include "Height.hpp"
#include <Mlib/Osm_Loader/Osm_Map_Resource/Osm_Map_Resource_Helpers.hpp>

using namespace Mlib;

HeightReference Mlib::parse_height_reference(const std::string& s) {
    static const std::map<std::string, HeightReference> m{
        { "ground", HeightReference::GROUND },
        { "water", HeightReference::WATER }
    };
    auto it = m.find(s);
    if (it == m.end()) {
        THROW_OR_ABORT("Unknown height reference: \"" + s + '"');
    }
    return it->second;
}

std::optional<HeightWithReference> Mlib::parse_height_with_reference(
    const std::map<std::string, std::string>& tags,
    const std::string& height_key,
    const std::string& reference_key,
    const std::string& object_name)
{
    auto height = parse_meters(tags, height_key, NAN);
    auto ref_it = tags.find(reference_key);
    if (std::isnan(height)) {
        if (ref_it != tags.end()) {
            THROW_OR_ABORT(object_name + ": Key \"" + reference_key + "\" requires \"" + height_key + '"');
        }
        return std::nullopt;
    }
    if (ref_it == tags.end()) {
        THROW_OR_ABORT(object_name + ": Key \"" + height_key + "\" requires \"" + reference_key + '"');
    }
    if (std::isnan(height)) {
        return std::nullopt;
    }
    return HeightWithReference{
        .height = height,
        .reference = parse_height_reference(ref_it->second)
    };
}
