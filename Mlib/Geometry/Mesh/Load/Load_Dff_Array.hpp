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
class DrawDistanceDb;

template <class TPosition>
struct DffArrays {
    std::list<std::shared_ptr<ColoredVertexArray<TPosition>>> renderables;
};

template <class TPosition>
DffArrays<TPosition> load_dff(
    const std::string& filename,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb);

template <class TPosition>
DffArrays<TPosition> load_dff(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TPosition>& cfg,
    const DrawDistanceDb& dddb);

}
