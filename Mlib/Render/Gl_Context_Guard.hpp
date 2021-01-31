#pragma once

struct GLFWwindow;

namespace Mlib {

class GlContextGuard {
public:
    GlContextGuard(GLFWwindow* window);
    ~GlContextGuard();
};

}
