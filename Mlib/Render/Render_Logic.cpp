#include "Render_Logic.hpp"
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

RenderLogic::RenderLogic() = default;

RenderLogic::~RenderLogic() = default;

FocusFilter RenderLogic::focus_filter() const {
    return { .focus_mask = Focus::ALWAYS };
}

float RenderLogic::near_plane() const {
    THROW_OR_ABORT("near_plane not implemented");
}

float RenderLogic::far_plane() const {
    THROW_OR_ABORT("far_plane not implemented");
}

const FixedArray<ScenePos, 4, 4>& RenderLogic::vp() const {
    THROW_OR_ABORT("vp not implemented");
}

const TransformationMatrix<float, ScenePos, 3>& RenderLogic::iv() const {
    THROW_OR_ABORT("iv not implemented");
}

DanglingPtr<const SceneNode> RenderLogic::camera_node() const {
    THROW_OR_ABORT("camera_node not implemented");
}

bool RenderLogic::requires_postprocessing() const {
    THROW_OR_ABORT("requires_postprocessing not implemented");
}

void RenderLogic::render_toplevel(
    const LayoutConstraintParameters& lx,
    const LayoutConstraintParameters& ly,
    const RenderConfig& render_config,
    const SceneGraphConfig& scene_graph_config,
    RenderResults* render_results,
    const RenderedSceneDescriptor& frame_id)
{
    DestructionGuard dg{ [this]() {
        reset();
        } };
    init(lx, ly, frame_id);
    render(lx, ly, render_config, scene_graph_config, render_results, frame_id);
}

// void RenderLogic::init(
//     const LayoutConstraintParameters& lx,
//     const LayoutConstraintParameters& ly,
//     const RenderedSceneDescriptor& frame_id)
// {
//     THROW_OR_ABORT("init not implemented");
// }

// void RenderLogic::reset() {
//     THROW_OR_ABORT("reset not implemented");
// }

// void RenderLogic::print(std::ostream& ostr, size_t depth) const {
//     THROW_OR_ABORT("print not implemented");
// }
