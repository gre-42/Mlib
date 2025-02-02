#pragma once
#include <Mlib/Render/Render_Logic.hpp>
#include <Mlib/Variable_And_Hash.hpp>
#include <memory>
#include <string>

namespace Mlib {

struct RenderingContext;
class RenderingResources;

class DirtmapLogic: public RenderLogic {
public:
    explicit DirtmapLogic(
        RenderingResources& rendering_resources,
        RenderLogic& child_logic);
    ~DirtmapLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual void render_without_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderingResources& rendering_resources_;
    RenderLogic& child_logic_;
    bool generated_;
    VariableAndHash<std::string> dirtmap_;
};

}
