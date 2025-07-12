#include "Bloom_Selector_Logic.hpp"
#include <Mlib/Log.hpp>
#include <Mlib/Render/Render_Logics/Bloom/Bloom_Mode.hpp>
#include <Mlib/Render/Render_Setup.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

BloomSelectorLogic::BloomSelectorLogic(
    RenderLogic& bloom_logic,
    RenderLogic& sky_bloom_logic,
    BloomMode mode)
    : bloom_logic_{ bloom_logic }
    , sky_bloom_logic_{ sky_bloom_logic }
    , mode_{ mode }
{}

BloomSelectorLogic::~BloomSelectorLogic() {
    on_destroy.clear();
}

std::optional<RenderSetup> BloomSelectorLogic::try_render_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderedSceneDescriptor& frame_id) const
{
    return child_logic().try_render_setup(lx, ly, frame_id);
}

bool BloomSelectorLogic::render_optional_setup(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id,
    const RenderSetup* setup)
{
    LOG_FUNCTION("BloomSelectorLogic::render");
    return child_logic().render_optional_setup(
        lx,
        ly,
        render_config,
        scene_graph_config,
        render_results,
        frame_id,
        setup);
}

RenderLogic& BloomSelectorLogic::child_logic() {
    switch (mode_) {
    case BloomMode::STANDARD:
        return bloom_logic_;
    case BloomMode::SKY:
        return sky_bloom_logic_;
    }
    THROW_OR_ABORT("Unknown bloom mode");
}

const RenderLogic& BloomSelectorLogic::child_logic() const {
    return const_cast<BloomSelectorLogic*>(this)->child_logic();
}

void BloomSelectorLogic::print(std::ostream& ostr, size_t depth) const {
    ostr << std::string(depth, ' ') << "BloomSelectorLogic\n";
    child_logic().print(ostr, depth + 1);
}
