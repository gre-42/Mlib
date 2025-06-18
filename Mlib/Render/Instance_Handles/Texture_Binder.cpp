#include "Texture_Binder.hpp"
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

TextureBinder::TextureBinder(GLint first_slot)
    : slot_{ first_slot }
{}

void TextureBinder::bind(GLint uniform, const ITextureHandle& handle) {
    CHK(glUniform1i(uniform, slot_));
    CHK(glActiveTexture(GL_TEXTURE0 + slot_));
    if (handle.layers() > 1) {
        if (handle.mipmap_mode() == MipmapMode::WITH_MIPMAPS_2D) {
            CHK(glBindTexture(GL_TEXTURE_2D_ARRAY, handle.handle<GLuint>()));
        } else {
            CHK(glBindTexture(GL_TEXTURE_3D, handle.handle<GLuint>()));
        }
    } else {
        CHK(glBindTexture(GL_TEXTURE_2D, handle.handle<GLuint>()));
    }
    ++slot_;
}
