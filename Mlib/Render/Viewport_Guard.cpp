#ifdef __ANDROID__
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#endif

#include "Viewport_Guard.hpp"

#include <Mlib/Render/CHK.hpp>
#include <cmath>

using namespace Mlib;

std::atomic<ViewportGuard*> ViewportGuard::current_guard_ = nullptr;

ViewportGuard::ViewportGuard(
    float x,
    float y,
    float width,
    float height)
: width{width},
  height{height},
  viewport_{x, y, width, height},
  prev_guard_{current_guard_}
{
    current_guard_ = this;
#ifdef __ANDROID__
    CHK(glViewport((GLint)x, (GLint)y, (GLint)width, (GLint)height));
#else
    CHK(glViewportIndexedf(0, x, y, width, height));
#endif
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
    float xx = std::isnan(x) ? 0.f : x >= 0 ? x : (float)screen_width + x;
    float yy = std::isnan(y) ? 0.f : y >= 0 ? y : (float)screen_height + y;
    float ww = std::isnan(width) ? (float)screen_width - xx : width;
    float hh = std::isnan(height) ? (float)screen_height - yy : height;
    if ((ww > 0) && (hh > 0)) {
        return std::make_optional<ViewportGuard>(xx, yy, ww, hh);
    } else {
        return std::nullopt;
    }
}

ViewportGuard::~ViewportGuard() {
    current_guard_ = prev_guard_;
    if (prev_guard_ != nullptr) {
        const auto& t = prev_guard_->viewport_;
#ifdef __ANDROID__
        CHK(glViewport((GLint)std::get<0>(t), (GLint)std::get<1>(t), (GLint)std::get<2>(t), (GLint)std::get<3>(t)));
#else
        CHK(glViewportIndexedf(0, std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)));
#endif
    }
}
