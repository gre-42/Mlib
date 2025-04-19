#pragma once
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <compare>
#include <list>
#include <map>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
class GroupAndName;
struct MaterialAndMorphology {
    Material material;
    Morphology morphology;
    std::partial_ordering operator <=> (const MaterialAndMorphology&) const = default;
};

template <class TPos>
std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> group_meshes_by_material(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas);

}
