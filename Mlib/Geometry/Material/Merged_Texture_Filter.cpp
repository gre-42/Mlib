#include "Merged_Texture_Filter.hpp"
#include <Mlib/Geometry/Material/Colormap_With_Modifiers.hpp>
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>

using namespace Mlib;

MergedTextureFilter::~MergedTextureFilter() = default;

bool MergedTextureFilter::matches(const MergedTextureName& merged_texture_name) const {
    return
        Mlib::re::regex_search((const std::string&)merged_texture_name.colormap.filename, included_names) &&
        !Mlib::re::regex_search((const std::string&)merged_texture_name.colormap.filename, excluded_names);
}
