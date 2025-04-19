#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <string>

namespace Mlib {

class IImposters;
class ISupplyDepots;
struct RenderableResourceFilter;
class RenderingResources;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
class Scene;
template <class T>
class VariableAndHash;

struct RootInstantiationOptions {
    RenderingResources* rendering_resources = nullptr;
    IImposters* imposters = nullptr;
    ISupplyDepots* supply_depots = nullptr;
    const VariableAndHash<std::string>& instance_name;
    const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix;
    Scene& scene;
    uint32_t max_imposter_texture_size = 0;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
