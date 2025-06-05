#include "Merge_Meshes.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
std::shared_ptr<ColoredVertexArray<TPos>> Mlib::merge_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology)
{
    if (cvas.empty()) {
        THROW_OR_ABORT("Attempt to merge empty list of meshes");
    }
    std::list<FixedArray<ColoredVertex<TPos>, 3>> triangles;
    std::list<FixedArray<uint8_t, 3>> discrete_triangle_texture_layers;
    std::list<FixedArray<float, 3>> alpha;
    std::list<FixedArray<float, 4>> interiormap_uvmaps;
    for (const auto& cva : cvas) {
        if (!cva->discrete_triangle_texture_layers.empty() &&
            (cva->discrete_triangle_texture_layers.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("merge_meshes: discrete_triangle_texture_layers size mismatch");
        }
        if (!cva->alpha.empty() &&
            (cva->alpha.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("merge_meshes: alpha size mismatch");
        }
        if (!cva->interiormap_uvmaps.empty() &&
            (cva->interiormap_uvmaps.size() != cva->triangles.size()))
        {
            THROW_OR_ABORT("merge_meshes: interiormap_uvmaps size mismatch");
        }
        for (const auto& t : cva->triangles) {
            triangles.push_back(t);
        }
        for (const auto& t : cva->discrete_triangle_texture_layers) {
            discrete_triangle_texture_layers.push_back(t);
        }
        for (const auto& t : cva->alpha) {
            alpha.push_back(t);
        }
        for (const auto& t : cva->interiormap_uvmaps) {
            interiormap_uvmaps.push_back(t);
        }
    }
    return std::make_shared<ColoredVertexArray<TPos>>(
        name,
        material,
        morphology,
        ModifierBacklog{},
        UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
        UUVector<FixedArray<ColoredVertex<TPos>, 3>>(triangles.begin(), triangles.end()),
        UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
        UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
        UUVector<FixedArray<float, 3>>{},
        UUVector<FixedArray<uint8_t, 3>>{discrete_triangle_texture_layers.begin(), discrete_triangle_texture_layers.end()},
        std::vector<UUVector<FixedArray<float, 3, 2>>>{},
        std::vector<UUVector<FixedArray<float, 3>>>{},
        UUVector<FixedArray<float, 3>>(alpha.begin(), alpha.end()),
        UUVector<FixedArray<float, 4>>(interiormap_uvmaps.begin(), interiormap_uvmaps.end()));
}

template std::shared_ptr<ColoredVertexArray<float>> Mlib::merge_meshes<float>(
    const std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology);
template std::shared_ptr<ColoredVertexArray<CompressedScenePos>> Mlib::merge_meshes<CompressedScenePos>(
    const std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology);
