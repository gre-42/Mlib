#pragma once
#include <cmath>
#include <cstdint>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Material;
struct SceneGraphConfig;
struct ExternalRenderPass;

class VisibilityCheck {
public:
    explicit VisibilityCheck(const FixedArray<float, 4, 4>& mvp);
    bool is_visible(
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        float max_distance,
        bool has_instances) const;
    float sorting_key(const Material& m) const;
    bool orthographic() const;
private:
    const FixedArray<float, 4, 4>& mvp_;
    bool orthographic_;
};

}
