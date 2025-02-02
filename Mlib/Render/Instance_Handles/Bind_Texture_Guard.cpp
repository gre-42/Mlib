#include "Bind_Texture_Guard.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

BindTextureGuard::BindTextureGuard(GLenum target, GLuint texture)
    : target_{ target }
{
    CHK(glBindTexture(target_, texture));
}

BindTextureGuard::~BindTextureGuard() {
    ABORT(glBindTexture(target_, 0));
}
