#include "Group_Meshes_By_Material.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <Mlib/Geometry/Mesh/Modifiers/Material_And_Morphology.hpp>

using namespace Mlib;

template <class TPos>
std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> Mlib::group_meshes_by_material(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas)
{
    std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> result;
    for (const auto& cva : cvas) {
        result[{cva->material, cva->morphology}].push_back(cva);
    }
    return result;
}

template std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<float>>>>
    Mlib::group_meshes_by_material(const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas);
template std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>>
    Mlib::group_meshes_by_material(const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas);
