#pragma once
#include <list>
#include <memory>
#include <string>

namespace Mlib {

struct AnimatedColoredVertexArrays;
template <class TPos>
struct LoadMeshConfig;

std::shared_ptr<AnimatedColoredVertexArrays> load_mhx2(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg);

}
