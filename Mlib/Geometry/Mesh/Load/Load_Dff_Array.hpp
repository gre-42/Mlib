#pragma once
#include <iosfwd>
#include <map>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;

std::map<std::string, std::shared_ptr<ColoredVertexArray<float>>> load_dff(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg);

std::map<std::string, std::shared_ptr<ColoredVertexArray<float>>> load_dff(
    std::istream& istr,
    const LoadMeshConfig<float>& cfg);

}
