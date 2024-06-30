#pragma once
#include <iosfwd>
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;

std::list<std::shared_ptr<ColoredVertexArray<float>>> load_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg);

std::list<std::shared_ptr<ColoredVertexArray<float>>> load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<float>& cfg);

}
