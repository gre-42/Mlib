#pragma once
#include <Mlib/Render/Render_Logic.hpp>

namespace Mlib {

enum class BloomMode;

class BloomSelectorLogic final: public RenderLogic {
public:
    BloomSelectorLogic(
        RenderLogic& bloom_logic,
        RenderLogic& sky_bloom_logic,
        BloomMode mode);
    ~BloomSelectorLogic();

    virtual std::optional<RenderSetup> try_render_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderedSceneDescriptor& frame_id) const override;
    virtual bool render_optional_setup(
        const LayoutConstraintParameters& lx,
        const LayoutConstraintParameters& ly,
        const RenderConfig& render_config,
        const SceneGraphConfig& scene_graph_config,
        RenderResults* render_results,
        const RenderedSceneDescriptor& frame_id,
        const RenderSetup* setup) override;
    virtual void print(std::ostream& ostr, size_t depth) const override;

private:
    RenderLogic& child_logic();
    const RenderLogic& child_logic() const;
    RenderLogic& bloom_logic_;
    RenderLogic& sky_bloom_logic_;
    BloomMode mode_;
};

}
