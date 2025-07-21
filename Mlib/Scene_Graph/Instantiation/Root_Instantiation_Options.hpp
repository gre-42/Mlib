#pragma once
#include <Mlib/Geometry/Material/Blend_Distances.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <list>
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
    std::list<VariableAndHash<std::string>>* instantiated_nodes = nullptr;
    const VariableAndHash<std::string>& instance_name;
    const TransformationMatrix<SceneDir, ScenePos, 3>& absolute_model_matrix;
    Scene& scene;
    uint32_t max_imposter_texture_size = 0;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
