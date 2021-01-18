#include "Render_Logic.hpp"
#include <Mlib/Scene_Graph/Focus.hpp>

using namespace Mlib;

Focus RenderLogic::focus_mask() const {
    return Focus::ALWAYS;
}

float RenderLogic::near_plane() const {
    throw std::runtime_error("near_plane not implemented");
}

float RenderLogic::far_plane() const  {
    throw std::runtime_error("far_plane not implemented");
}

const FixedArray<float, 4, 4>& RenderLogic::vp() const  {
    throw std::runtime_error("vp not implemented");
}

const TransformationMatrix<float, 3>& RenderLogic::iv() const  {
    throw std::runtime_error("iv not implemented");
}

bool RenderLogic::requires_postprocessing() const  {
    throw std::runtime_error("requires_postprocessing not implemented");
}
