
#include "Colored_Vertex_Array_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>

using namespace Mlib;

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(included_tags);
DECLARE_ARGUMENT(excluded_tags);
DECLARE_ARGUMENT(included_names);
DECLARE_ARGUMENT(excluded_names);
}

ColoredVertexArrayFilter::~ColoredVertexArrayFilter() = default;

bool ColoredVertexArrayFilter::matches(const MeshMeta& meta) const {
    auto n = meta.name.name();
    return
        !any(~meta.morphology.physics_material & included_tags) &&
        !any(meta.morphology.physics_material & excluded_tags) &&
        Mlib::re::regex_search(n.data(), n.data() + n.size(), included_names) &&
        !Mlib::re::regex_search(n.data(), n.data() + n.size(), excluded_names);
}

template <class TPos>
bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<TPos>& cva) const {
    return matches(cva.meta);
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

bool ColoredVertexArrayFilters::matches(const MeshMeta& meta) const {
    for (const auto& f : filters_) {
        if (f.matches(meta)) {
            return true;
        }
    }
    return false;
}

template <class TPos>
bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<TPos>& cva) const {
    return matches(cva.meta);
}

void Mlib::from_json(const nlohmann::json& j, ColoredVertexArrayFilters& filters) {
    if (j.type() != nlohmann::detail::value_t::array) {
        throw std::runtime_error("Type is not array for ColoredVertexArrayFilters");
    }
    filters = ColoredVertexArrayFilters{ j.get<std::vector<ColoredVertexArrayFilter>>() };
}

template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilter::matches(const ColoredVertexArray<CompressedScenePos>& cva) const;

template bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<float>& cva) const;
template bool ColoredVertexArrayFilters::matches(const ColoredVertexArray<CompressedScenePos>& cva) const;
