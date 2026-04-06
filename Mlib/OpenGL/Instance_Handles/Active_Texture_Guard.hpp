#pragma once
#include <Mlib/OpenGL/Any_Gl.hpp>

namespace Mlib {

class ActiveTextureGuard {
    ActiveTextureGuard(const ActiveTextureGuard&) = delete;
    ActiveTextureGuard& operator = (const ActiveTextureGuard&) = delete;
public:
    explicit ActiveTextureGuard(GLenum texture_unit);
    ~ActiveTextureGuard();
};

}
