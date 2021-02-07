#pragma once
#include <Mlib/Geometry/Homogeneous.hpp>
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Graph/Render_Pass.hpp>
#include <Mlib/Scene_Graph/Scene_Graph_Config.hpp>

namespace Mlib {

class VisibilityCheck {
public:
    inline explicit VisibilityCheck(const FixedArray<float, 4, 4>& mvp)
    : mvp_{mvp},
      orthographic_{(mvp(3, 0) == 0 && mvp(3, 1) == 0 && mvp(3, 2) == 0 && mvp(3, 3) == 1)}
    {}
    inline bool is_visible(
        const Material& m,
        const SceneGraphConfig& scene_graph_config,
        const ExternalRenderPass& external_render_pass,
        float max_distance = NAN) const
    {
        if ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && (m.occluder_type != OccluderType::OFF)) {
            return true;
        }
        if (std::isnan(max_distance) && !m.is_small) {
            return true;
        }
        if (orthographic_) {
            return false;
        }
        float max_dist2 = std::isnan(max_distance)
            ? squared(scene_graph_config.max_distance_small)
            : squared(max_distance);
        return sum(squared(t3_from_4x4(mvp_))) < max_dist2;
    }
    inline float sorting_key(const Material& m, const SceneGraphConfig& scene_graph_config) const {
        return ((m.blend_mode == BlendMode::CONTINUOUS) && !orthographic_)
            ? -mvp_(2, 3)
            : -INFINITY;
    }
    inline bool orthographic() {
        return orthographic_;
    }
private:
    const FixedArray<float, 4, 4>& mvp_;
    bool orthographic_;
};

}
