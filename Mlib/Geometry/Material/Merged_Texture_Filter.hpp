#pragma once
#include <Mlib/Regex/Default_Regex.hpp>
#include <Mlib/Regex/Regex_Select.hpp>

namespace Mlib {

struct MergedTextureName;

struct MergedTextureFilter {
    ~MergedTextureFilter();
    Mlib::regex included_names = ALWAYS;
    Mlib::regex excluded_names = NEVER;
    bool matches(const MergedTextureName& merged_texture_name) const;
};

}
