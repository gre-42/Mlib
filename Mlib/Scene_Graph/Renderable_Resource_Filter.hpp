#pragma once
#include <Mlib/Regex_Select.hpp>
#include <Mlib/Scene_Graph/Resource_Filter.hpp>

namespace Mlib {

struct ColoredVertexArray;

struct RenderableResourceFilter {
    size_t min_num = 0;
    size_t max_num = SIZE_MAX;
    ResourceFilter resource_filter;
    bool matches(size_t num, const ColoredVertexArray& cva) const;
};

}
