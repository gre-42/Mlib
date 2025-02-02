#include "Viewport_Guard.hpp"
#include <Mlib/Layout/IWidget.hpp>
#include <Mlib/Memory/Float_To_Integral.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <cmath>

using namespace Mlib;

std::atomic<ViewportGuard*> ViewportGuard::current_guard_ = nullptr;

ViewportGuard::ViewportGuard(
    int width,
    int height)
: ViewportGuard(
    0.f,
    0.f,
    (float)width,
    (float)height)
{}

std::optional<ViewportGuard> ViewportGuard::from_widget(
    const IPixelRegion &ew)
{
    if ((ew.width() > 0) && (ew.height() > 0)) {
        return std::make_optional<ViewportGuard>(
            ew.left(),
            ew.bottom(),
            ew.width(),
            ew.height());
    } else {
        return std::nullopt;
    }
}

std::optional<ViewportGuard> ViewportGuard::periodic(
    float x,
    float y,
    float width,
    float height,
    int screen_width,
    int screen_height)
{
    float xx = std::isnan(x) ? 0.f : !std::signbit(x) ? x : (float)screen_width + x;
    float yy = std::isnan(y) ? 0.f : !std::signbit(y) ? y : (float)screen_height + y;
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
        prev_guard_->apply();
    }
}

#ifdef __ANDROID__
struct IntViewport {
    IntViewport(float begin, float size) {
        ibegin = float_to_integral<int>(begin);
        int iend = ibegin + float_to_integral<int>(size);
        isize = iend - ibegin;
    }
    int ibegin;
    int isize;
};

ViewportGuard::ViewportGuard(
    float x,
    float y,
    float width,
    float height)
: prev_guard_{current_guard_}
{
    current_guard_ = this;
    IntViewport vx{x, width};
    IntViewport vy{y, height};
    viewport_.x = vx.ibegin;
    viewport_.y = vy.ibegin;
    viewport_.width = vx.isize;
    viewport_.height = vy.isize;
    apply();
}

void ViewportGuard::apply() const {
    CHK(glViewport(viewport_.x, viewport_.y, viewport_.width, viewport_.height));
}

float ViewportGuard::fwidth() const {
    return (float)viewport_.width;
}

float ViewportGuard::fheight() const {
    return (float)viewport_.height;
}

int ViewportGuard::iwidth() const {
    return viewport_.width;
}

int ViewportGuard::iheight() const {
    return viewport_.height;
}

#else

ViewportGuard::ViewportGuard(
    float x,
    float y,
    float width,
    float height)
    : viewport_{ x, y, width, height }
    , prev_guard_{ current_guard_ }
{
    current_guard_ = this;
    apply();
}

void ViewportGuard::apply() const {
    CHK(glViewportIndexedf(0, viewport_.x, viewport_.y, viewport_.width, viewport_.height));
}

float ViewportGuard::fwidth() const {
    return viewport_.width;
}

float ViewportGuard::fheight() const {
    return viewport_.height;
}

int ViewportGuard::iwidth() const {
    return float_to_integral<int>(viewport_.width);
}

int ViewportGuard::iheight() const {
    return float_to_integral<int>(viewport_.height);
}

#endif
