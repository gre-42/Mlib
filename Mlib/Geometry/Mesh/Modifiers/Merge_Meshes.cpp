#include "Merge_Meshes.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>
#include <list>

using namespace Mlib;

template <class TPos>
void Mlib::merge_meshes(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology)
{
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>> result;
    if (!cvas.empty()) {
        std::list<FixedArray<ColoredVertex<TPos>, 3>> triangles;
        for (const auto& cva : cvas) {
            for (const auto& t : cva->triangles) {
                triangles.push_back(t);
            }
        }
        result.push_back(std::make_shared<ColoredVertexArray<TPos>>(
            name,
            material,
            morphology,
            ModifierBacklog{},
            UUVector<FixedArray<ColoredVertex<TPos>, 4>>{},
            UUVector<FixedArray<ColoredVertex<TPos>, 3>>(triangles.begin(), triangles.end()),
            UUVector<FixedArray<ColoredVertex<TPos>, 2>>{},
            UUVector<FixedArray<std::vector<BoneWeight>, 3>>{},
            UUVector<FixedArray<float, 3>>{},
            UUVector<FixedArray<uint8_t, 3>>{},
            std::vector<UUVector<FixedArray<float, 3, 2>>>{},
            std::vector<UUVector<FixedArray<float, 3>>>{},
            UUVector<FixedArray<float, 3>>{}));
    }
    cvas = std::move(result);
}

template void Mlib::merge_meshes<float>(
    std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology);
template void Mlib::merge_meshes<CompressedScenePos>(
    std::list<std::shared_ptr<ColoredVertexArray<CompressedScenePos>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology);
