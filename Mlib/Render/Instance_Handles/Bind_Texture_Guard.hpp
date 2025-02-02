#pragma once
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class BindTextureGuard {
    BindTextureGuard(const BindTextureGuard&) = delete;
    BindTextureGuard& operator = (const BindTextureGuard&) = delete;
public:
    explicit BindTextureGuard(GLenum target, GLuint texture);
    ~BindTextureGuard();
private:
    GLenum target_;
};

}
