#pragma once
#include <Mlib/Geometry/Physics_Material.hpp>
#include <Mlib/Json/Base.hpp>
#include <Mlib/Regex/Default_Regex.hpp>
#include <Mlib/Regex/Regex_Select.hpp>
#include <vector>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;

struct ColoredVertexArrayFilter {
    ~ColoredVertexArrayFilter();
    PhysicsMaterial included_tags = PhysicsMaterial::NONE;
    PhysicsMaterial excluded_tags = PhysicsMaterial::NONE;
    Mlib::re::cregex included_names = ALWAYS;
    Mlib::re::cregex excluded_names = NEVER;
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
};

class ColoredVertexArrayFilters {
public:
    ColoredVertexArrayFilters();
    explicit ColoredVertexArrayFilters(std::vector<ColoredVertexArrayFilter> filters);
    template <class TPos>
    bool matches(const ColoredVertexArray<TPos>& cva) const;
private:
    std::vector<ColoredVertexArrayFilter> filters_;
};

void from_json(const nlohmann::json& j, ColoredVertexArrayFilter& filter);
void from_json(const nlohmann::json& j, ColoredVertexArrayFilters& filters);

}
