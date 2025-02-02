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
class IRaceLogic;

template <class TPos>
std::list<std::shared_ptr<ColoredVertexArray<TPos>>> load_kn5_array(
    const std::string& filename,
    const LoadMeshConfig<TPos>& cfg,
    IDdsResources* dds_resources,
    IRaceLogic* race_logic);

}
