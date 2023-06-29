#pragma once
#include <Mlib/Regex/Regex_Select.hpp>

namespace Mlib {

struct MergedTextureName;

struct MergedTextureFilter {
    ~MergedTextureFilter();
    DECLARE_REGEX(included_names, "");
    DECLARE_REGEX(excluded_names, "$ ^");
    bool matches(const MergedTextureName& merged_texture_name) const;
};

}
