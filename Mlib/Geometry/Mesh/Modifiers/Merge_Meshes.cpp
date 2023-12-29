#include "Merge_Meshes.hpp"
#include <Mlib/Geometry/Colored_Vertex.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Mesh/Colored_Vertex_Array.hpp>

using namespace Mlib;

template <class TPos>
void Mlib::merge_meshes(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::string& name,
    const Material& material,
    const PhysicsMaterial physics_material)
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
            physics_material,
            ModifierBacklog{},
            std::vector<FixedArray<ColoredVertex<TPos>, 4>>{},
            std::vector<FixedArray<ColoredVertex<TPos>, 3>>(triangles.begin(), triangles.end()),
            std::vector<FixedArray<ColoredVertex<TPos>, 2>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 3>>{},
            std::vector<FixedArray<std::vector<BoneWeight>, 2>>{},
            std::vector<FixedArray<uint8_t, 3>>{},
            std::vector<FixedArray<uint8_t, 2>>{}));
    }
    cvas = std::move(result);
}

namespace Mlib {
    template void merge_meshes<float>(
        std::list<std::shared_ptr<ColoredVertexArray<float>>>& cvas,
        const std::string& name,
        const Material& material,
        const PhysicsMaterial physics_material);
    template void merge_meshes<double>(
        std::list<std::shared_ptr<ColoredVertexArray<double>>>& cvas,
        const std::string& name,
        const Material& material,
        const PhysicsMaterial physics_material);
}
