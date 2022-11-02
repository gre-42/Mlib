#pragma once

struct GLFWwindow;

namespace Mlib {

class GlContextGuard {
    GlContextGuard(const GlContextGuard&) = delete;
    GlContextGuard& operator = (const GlContextGuard&) = delete;
public:
    GlContextGuard(GLFWwindow* window);
    ~GlContextGuard();
};

}
