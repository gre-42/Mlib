#include "Viewport_Guard.hpp"

using namespace Mlib;

std::list<std::tuple<GLint, GLint, GLsizei, GLsizei>> ViewportGuard::stack_;

ViewportGuard::ViewportGuard(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height)
{
    stack_.push_back({x, y, width, height});
    CHK(glViewport(x, y, width, height));
}

ViewportGuard::ViewportGuard(
    GLint x,
    GLint y,
    GLsizei width,
    GLsizei height,
    GLsizei screen_width,
    GLsizei screen_height,
    Periodicity position_periodicity)
: ViewportGuard(
    (position_periodicity == Periodicity::APERIODIC) || x >= 0 ? x : screen_width + x,
    (position_periodicity == Periodicity::APERIODIC) || y >= 0 ? y : screen_height + y,
    width != -1 ? width : screen_width,
    height != -1 ? height : screen_height)
{}

ViewportGuard::~ViewportGuard() {
    stack_.pop_back();
    if (!stack_.empty()) {
        const auto& t = stack_.back();
        CHK(glViewport(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)));
    }
}
