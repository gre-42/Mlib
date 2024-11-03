#pragma once
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Map/Unordered_Map.hpp>
#include <iosfwd>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;

template <class TResourcePos, class TInstancePos>
struct PssgArrays {
    UnorderedMap<std::string, std::shared_ptr<ColoredVertexArray<TResourcePos>>> resources;
    std::list<InstanceInformation<TInstancePos>> instances;
};

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> load_pssg_arrays(
    const std::string& filename,
    const LoadMeshConfig<TResourcePos>& cfg);

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> load_pssg_arrays(
    std::istream& istr,
    const std::string& name,
    const LoadMeshConfig<TResourcePos>& cfg);

}
