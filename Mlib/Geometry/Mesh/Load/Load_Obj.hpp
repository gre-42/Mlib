#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> load_obj(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg);

}
