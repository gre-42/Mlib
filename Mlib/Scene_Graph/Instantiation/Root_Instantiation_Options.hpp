#pragma once
#include <Mlib/Scene_Pos.hpp>
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

struct RootInstantiationOptions {
    RenderingResources* rendering_resources = nullptr;
    IImposters* imposters = nullptr;
    ISupplyDepots* supply_depots = nullptr;
    const std::string& instance_name;
    const TransformationMatrix<float, ScenePos, 3>& absolute_model_matrix;
    Scene& scene;
    const RenderableResourceFilter& renderable_resource_filter;
};

}
