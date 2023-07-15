#pragma once
#include <list>
#include <memory>

namespace Mlib {

struct AnimatedColoredVertexArrays;
template <class TPos>
struct LoadMeshConfig;

std::shared_ptr<AnimatedColoredVertexArrays> load_mhx2(
    const std::string& filename,
    const LoadMeshConfig<float>& cfg);

}
