#include "Render_Logic.hpp"
#include <Mlib/Scene_Graph/Focus.hpp>
#include <Mlib/Scene_Graph/Focus_Filter.hpp>
#include <stdexcept>

using namespace Mlib;

RenderLogic::~RenderLogic()
{}

FocusFilter RenderLogic::focus_filter() const {
    return { .focus_mask = Focus::ALWAYS };
}

float RenderLogic::near_plane() const {
    throw std::runtime_error("near_plane not implemented");
}

float RenderLogic::far_plane() const {
    throw std::runtime_error("far_plane not implemented");
}

const FixedArray<float, 4, 4>& RenderLogic::vp() const {
    throw std::runtime_error("vp not implemented");
}

const TransformationMatrix<float, 3>& RenderLogic::iv() const {
    throw std::runtime_error("iv not implemented");
}

const SceneNode& RenderLogic::camera_node() const {
    throw std::runtime_error("camera_node not implemented");
}

bool RenderLogic::requires_postprocessing() const {
    throw std::runtime_error("requires_postprocessing not implemented");
}

// void RenderLogic::print(std::ostream& ostr, size_t depth) const {
//     throw std::runtime_error("print not implemented");
// }
