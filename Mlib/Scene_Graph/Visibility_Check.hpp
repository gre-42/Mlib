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
    inline bool is_visible(const Material& m, const SceneGraphConfig& scene_graph_config, const ExternalRenderPass& external_render_pass) const {
        return ((!m.is_small) ||
                ((external_render_pass.pass == ExternalRenderPassType::LIGHTMAP_TO_TEXTURE) && (m.occluder_type != OccluderType::OFF)) ||
                (// (mvp(2, 3) > scene_graph_config.min_distance_small) && // no mvp-check to support rotations
                 !orthographic_ &&
                 (sum(squared(t3_from_4x4(mvp_))) > squared(scene_graph_config.min_distance_small)) &&
                 (sum(squared(t3_from_4x4(mvp_))) < squared(scene_graph_config.max_distance_small))));
        }
    inline bool orthographic() {
        return orthographic_;
    }
private:
    const FixedArray<float, 4, 4>& mvp_;
    bool orthographic_;
};

}
