#pragma once
#include <list>
#include <map>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
class GroupAndName;
struct MaterialAndMorphology;

template <class TPos>
std::map<MaterialAndMorphology, std::list<std::shared_ptr<ColoredVertexArray<TPos>>>> group_meshes_by_material(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas);

}
