#include "Group_Elements_By_Material.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Material_And_Morphology.hpp>
#include <Mlib/Geometry/Mesh/Triangle_List.hpp>

using namespace Mlib;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> Mlib::group_elements_by_material(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
{
    std::map<MaterialAndMorphology, TriangleList<TPos>> lists;
    for (const auto& cva : cvas) {
        auto& l = lists.try_emplace({cva->material, cva->morphology}, cva->name, cva->material, cva->morphology).first->second;
        l.triangles.insert(l.triangles.end(), cva->triangles.begin(), cva->triangles.end());
    }
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    for (const auto& [_, l] : lists) {
        result.emplace_back(l.triangle_array());
    }
    return result;
}

template std::list<std::shared_ptr<ColoredVertexArray<float>>>
    Mlib::group_elements_by_material(const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas);
template std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>
    Mlib::group_elements_by_material(const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas);
