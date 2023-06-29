#include "Merged_Texture_Filter.hpp"
#include <Mlib/Geometry/Material/Merged_Texture_Name.hpp>

using namespace Mlib;

MergedTextureFilter::~MergedTextureFilter() = default;

bool MergedTextureFilter::matches(const MergedTextureName& merged_texture_name) const {
    return
        Mlib::re::regex_search(merged_texture_name.name, included_names) &&
        !Mlib::re::regex_search(merged_texture_name.name, excluded_names);
}
