#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

template <class TPos>
class ColoredVertexArray;
template <class TPos>
struct LoadMeshConfig;
class IDdsResources;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> load_kn5_array(
    const std::string& file_or_directory,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources);

}
