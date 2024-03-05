#pragma once
#include <cstddef>
#include <list>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct Light;
struct Skidmark;
struct RenderConfig;
struct SceneGraphConfig;
struct ExternalRenderPass;
class ITrailStorage;

class ITrailRenderer {
public:
    virtual ~ITrailRenderer() = default;
    virtual void preload(const std::string& resource_name) = 0;
    virtual ITrailStorage& get_storage(const std::string& resource_name) = 0;
    virtual void move(float dt) = 0;
    virtual void render(
        const FixedArray<double, 4, 4>& vp,
        const TransformationMatrix<float, double, 3>& iv,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Light*>>& lights,
        const std::list<std::pair<TransformationMatrix<float, double, 3>, Skidmark*>>& skidmarks,
        const SceneGraphConfig& scene_graph_config,
        const RenderConfig& render_config,
        const ExternalRenderPass& external_render_pass) const = 0;
};

}
