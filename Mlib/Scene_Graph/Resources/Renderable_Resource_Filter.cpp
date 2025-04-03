#include "Renderable_Resource_Filter.hpp"
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Physics_Material.hpp>

using namespace Mlib;

RenderableResourceFilter::~RenderableResourceFilter()
{}

template <class TPos>
bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<TPos>& cva) const {
    return
        (num >= min_num) &&
        (num <= max_num) &&
        cva_filter.matches(cva);
}

std::ostream& Mlib::operator << (std::ostream& ostr, const RenderableResourceFilter& filter) {
    ostr << "RenderableResourceFilter\n";
    ostr << "  min_num: " << filter.min_num << '\n';
    ostr << "  max_num: " << filter.max_num << '\n';
    ostr << "  included tags: " << physics_material_to_string(filter.cva_filter.included_tags) << '\n';
    ostr << "  excluded tags: " << physics_material_to_string(filter.cva_filter.excluded_tags) << '\n';
    ostr << "  included names: <regex>\n";
    ostr << "  excluded names: <regex>\n";
    return ostr;
}

template bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<float>& cva) const;
template bool RenderableResourceFilter::matches(size_t num, const ColoredVertexArray<CompressedScenePos>& cva) const;
