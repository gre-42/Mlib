#pragma once

namespace Mlib {

struct LayoutConstraintParameters;
struct RenderConfig;
struct SceneGraphConfig;
struct RenderResults;
struct RenderedSceneDescriptor;
struct StaticWorld;

class VisualMovableLoggerView {
public:
    virtual ~VisualMovableLoggerView() = default;

    virtual void advance_time(float dt, const StaticWorld& world) = 0;

    virtual void render(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) = 0;
};

}
