#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include "Viewport_Guard.hpp"

#include <Mlib/Render/CHK.hpp>
#include <cmath>

using namespace Mlib;

std::list<std::tuple<float, float, float, float>> ViewportGuard::stack_;

ViewportGuard::ViewportGuard(
    float x,
    float y,
    float width,
    float height)
{
    CHK(glViewportIndexedf(0, x, y, width, height));
    stack_.push_back({x, y, width, height});
}

ViewportGuard::ViewportGuard(
    int width,
    int height)
: ViewportGuard(
    0.f,
    0.f,
    (float)width,
    (float)height)
{}

ViewportGuard::ViewportGuard(
    float x,
    float y,
    float width,
    float height,
    int screen_width,
    int screen_height,
    Periodicity position_periodicity)
: ViewportGuard(
    std::isnan(x) ? 0.f : (position_periodicity == Periodicity::APERIODIC) || x >= 0 ? x : screen_width + x,
    std::isnan(y) ? 0.f : (position_periodicity == Periodicity::APERIODIC) || y >= 0 ? y : screen_height + y,
    !std::isnan(width) ? width : screen_width,
    !std::isnan(height) ? height : screen_height)
{}

ViewportGuard::~ViewportGuard() {
    stack_.pop_back();
    if (!stack_.empty()) {
        const auto& t = stack_.back();
        CHK(glViewportIndexedf(0, std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)));
    }
}
