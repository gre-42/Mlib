#pragma once
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

struct ColoredVertexArray;

struct ResourceFilter {
    DECLARE_REGEX(include, "");
    DECLARE_REGEX(exclude, "$ ^");
    bool matches(const ColoredVertexArray& cva) const;
};

}
