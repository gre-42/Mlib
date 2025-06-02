#include "Active_Texture_Guard.hpp"
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

ActiveTextureGuard::ActiveTextureGuard(GLenum texture_unit) {
    CHK(glActiveTexture(texture_unit));
}

ActiveTextureGuard::~ActiveTextureGuard() {
    ABORT(glActiveTexture(GL_TEXTURE0));
}
