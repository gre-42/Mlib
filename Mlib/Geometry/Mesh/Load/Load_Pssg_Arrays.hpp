#pragma once
#include <Mlib/Geometry/Instance/Instance_Information.hpp>
#include <Mlib/Map/String_With_Hash_Unordered_Map.hpp>
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
class IDdsResources;
enum class IoVerbosity;
struct PssgModel;

template <class TResourcePos, class TInstancePos>
struct PssgArrays {
    PssgArrays()
        : resources{ "PSSG resources" }
    {}
    StringWithHashUnorderedMap<std::shared_ptr<ColoredVertexArray<TResourcePos>>> resources;
    std::list<InstanceInformation<TInstancePos>> instances;
};

template <class TResourcePos, class TInstancePos>
PssgArrays<TResourcePos, TInstancePos> load_pssg_arrays(
    const PssgModel& model,
    const LoadMeshConfig<TResourcePos>& cfg,
    IDdsResources* dds_resources,
    const std::string& resource_prefix,
    IoVerbosity verbosity);

}
