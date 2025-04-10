#include "Colored_Vertex_Array_Filter.hpp"
#include <Mlib/Argument_List.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Scene_Precision.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(included_tags);
DECLARE_ARGUMENT(excluded_tags);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

ColoredVertexArrayFilter::~ColoredVertexArrayFilter() = default;

template <class TPos>
bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<TPos>& cva) const {
    auto n = cva.name.name();
    return
        !any(~cva.morphology.physics_material & included_tags) &&
        !any(cva.morphology.physics_material & excluded_tags) &&
        Mlib::re::regex_search(n.data(), n.data() + n.size(), included_names) &&
        !Mlib::re::regex_search(n.data(), n.data() + n.size(), excluded_names);
}

void Mlib::from_json(const nlohmann::json& j, ColoredVertexArrayFilter& filter) {
    JsonView jv{ j };
    jv.validate(KnownArgs::options);
    if (jv.contains(KnownArgs::included_tags)) {
        filter.included_tags = physics_material_from_string(jv.at<std::string>(KnownArgs::included_tags));
    }
    if (jv.contains(KnownArgs::excluded_tags)) {
        filter.excluded_tags = physics_material_from_string(jv.at<std::string>(KnownArgs::excluded_tags));
    }
    if (jv.contains(KnownArgs::included_names)) {
        filter.included_names = Mlib::compile_regex(jv.at<std::string>(KnownArgs::included_names));
    }
    if (jv.contains(KnownArgs::excluded_names)) {
        filter.excluded_names = Mlib::compile_regex(jv.at<std::string>(KnownArgs::excluded_names));
    }
}

ColoredVertexArrayFilters::ColoredVertexArrayFilters()
: filters_{ ColoredVertexArrayFilter{} }
{}

ColoredVertexArrayFilters::ColoredVertexArrayFilters(std::vector<ColoredVertexArrayFilter> filters)
    : filters_{ std::move(filters) }
{}

template <class TPos>
bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<TPos>& cva) const {
    for (const auto& f : filters_) {
        if (f.matches(cva)) {
            return true;
        }
    }
    return false;
}

void Mlib::from_json(const nlohmann::json& j, ColoredVertexArrayFilters& filters) {
    if (j.type() != nlohmann::detail::value_t::array) {
        THROW_OR_ABORT("Type is not array for ColoredVertexArrayFilters");
    }
    filters = ColoredVertexArrayFilters{ j.get<std::vector<ColoredVertexArrayFilter>>() };
}

template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<CompressedScenePos>& cva) const;

template bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<CompressedScenePos>& cva) const;
