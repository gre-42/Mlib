#pragma once
#include <cmath>
#include <cstdint>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
struct Material;
struct SceneGraphConfig;
struct ExternalRenderPass;
enum class ExternalRenderPassType;

class VisibilityCheck {
public:
    explicit VisibilityCheck(const FixedArray<double, 4, 4>& mvp);
    bool is_visible(
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass) const;
    bool is_visible(double max_center_distance) const;
    bool black_is_visible(
        const Material& m,
        uint32_t billboard_id,
        const SceneGraphConfig& scene_graph_config,
        ExternalRenderPassType external_render_pass) const;
    double sorting_key(const Material& m) const;
    bool orthographic() const;
private:
    double distance_squared() const;
    const FixedArray<double, 4, 4>& mvp_;
    bool orthographic_;
};

}
