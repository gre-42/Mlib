#pragma once
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct MaterialAndMorphology;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> group_elements_by_material(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas);

}
