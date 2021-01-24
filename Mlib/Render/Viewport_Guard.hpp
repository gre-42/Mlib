#pragma once
#include <glad/gl.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <Mlib/Render/CHK.hpp>
#include <list>
#include <tuple>

namespace Mlib {

class ViewportGuard {
public:
    ViewportGuard(
        GLint x,
        GLint y,
        GLsizei width,
        GLsizei height);
    ~ViewportGuard();
private:
    static std::list<std::tuple<GLint, GLint, GLsizei, GLsizei>> stack_;
};

}
