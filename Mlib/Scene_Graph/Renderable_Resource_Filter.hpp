#pragma once
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

struct ColoredVertexArray;

struct RenderableResourceFilter {
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    DECLARE_REGEX(include, "");
    DECLARE_REGEX(exclude, "$ ^");
    bool matches(size_t num, const ColoredVertexArray& cva) const;
};

}
