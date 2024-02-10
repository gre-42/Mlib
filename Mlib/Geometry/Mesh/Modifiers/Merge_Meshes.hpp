#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct Material;
enum class PhysicsMaterial: uint32_t;

template <class TPos>
void merge_meshes(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::string& name,
    const Material& material,
    const PhysicsMaterial physics_material);

}
