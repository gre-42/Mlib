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
: width{width},
  height{height}
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

std::optional<ViewportGuard> ViewportGuard::periodic(
    float x,
    float y,
    float width,
    float height,
    int screen_width,
    int screen_height)
{
    float xx = std::isnan(x) ? 0.f : x >= 0 ? x : screen_width + x;
    float yy = std::isnan(y) ? 0.f : y >= 0 ? y : screen_height + y;
    float ww = std::isnan(width) ? screen_width - xx : width;
    float hh = std::isnan(height) ? screen_height - yy : height;
    if ((ww > 0) && (hh > 0)) {
        return std::make_optional<ViewportGuard>(xx, yy, ww, hh);
    } else {
        return std::nullopt;
    }
}

ViewportGuard::~ViewportGuard() {
    stack_.pop_back();
    if (!stack_.empty()) {
        const auto& t = stack_.back();
        CHK(glViewportIndexedf(0, std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)));
    }
}
