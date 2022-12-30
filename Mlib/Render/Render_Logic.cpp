#include "Render_Logic.hpp"
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

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

const FixedArray<double, 4, 4>& RenderLogic::vp() const {
    THROW_OR_ABORT("vp not implemented");
}

const TransformationMatrix<float, double, 3>& RenderLogic::iv() const {
    THROW_OR_ABORT("iv not implemented");
}

const SceneNode& RenderLogic::camera_node() const {
    THROW_OR_ABORT("camera_node not implemented");
}

bool RenderLogic::requires_postprocessing() const {
    THROW_OR_ABORT("requires_postprocessing not implemented");
}

// void RenderLogic::print(std::ostream& ostr, size_t depth) const {
//     THROW_OR_ABORT("print not implemented");
// }
