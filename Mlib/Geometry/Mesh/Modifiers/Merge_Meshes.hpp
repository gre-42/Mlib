#pragma once
#include <cstdint>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
struct Material;
struct Morphology;

template <class TPos>
void merge_meshes(
    std::list<std::shared_ptr<ColoredVertexArray<TPos>>>& cvas,
    const std::string& name,
    const Material& material,
    const Morphology& morphology);

}
