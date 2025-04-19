#pragma once
#include <cstdint>
#include <list>
#include <memory>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct Material;
struct Morphology;
class GroupAndName;

template <class TPos>
std::shared_ptr<ColoredVertexArray<TPos>> merge_meshes(
    const std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const GroupAndName& name,
    const Material& material,
    const Morphology& morphology);

}
