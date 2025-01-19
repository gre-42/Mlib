#pragma once
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Regex/Default_Regex.hpp>
#include <Mlib/Regex/Regex_Select.hpp>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct ColoredVertexArrayFilter {
    ~ColoredVertexArrayFilter();
    PhysicsMaterial included_tags = PhysicsMaterial::NONE;
    PhysicsMaterial excluded_tags = PhysicsMaterial::NONE;
    Mlib::regex included_names = ALWAYS;
    Mlib::regex excluded_names = NEVER;
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
};

}
