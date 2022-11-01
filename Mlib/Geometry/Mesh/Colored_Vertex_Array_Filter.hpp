#pragma once
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Regex_Select.hpp>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct ColoredVertexArrayFilter {
    PhysicsMaterial included_tags = PhysicsMaterial::NONE;
    PhysicsMaterial excluded_tags = PhysicsMaterial::NONE;
    DECLARE_REGEX(included_names, "");
    DECLARE_REGEX(excluded_names, "$ ^");
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
};

}
