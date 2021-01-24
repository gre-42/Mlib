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
    glViewport(x, y, width, height);
}

ViewportGuard::~ViewportGuard() {
    stack_.pop_back();
    if (!stack_.empty()) {
        const auto& t = stack_.back();
        CHK(glViewport(std::get<0>(t), std::get<1>(t), std::get<2>(t), std::get<3>(t)));
    }
}
