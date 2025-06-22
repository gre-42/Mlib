#include "Texture_Binder.hpp"
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TextureBinder::TextureBinder(GLint first_slot)
    : slot_{ first_slot }
{}

void TextureBinder::bind(GLint uniform, const ITextureHandle& handle) {
    if (uniform < 0) {
        THROW_OR_ABORT("Uniform ID is negative");
    }
    CHK(glUniform1i(uniform, slot_));
    CHK(glActiveTexture(GL_TEXTURE0 + slot_));
    GLenum target;
    if (handle.layers() > 1) {
        if (handle.mipmap_mode() == MipmapMode::WITH_MIPMAPS_2D) {
            target = GL_TEXTURE_2D_ARRAY;
        } else {
            target = GL_TEXTURE_3D;
        }
    } else {
        target = GL_TEXTURE_2D;
    }
    CHK(glBindTexture(target, handle.handle<GLuint>()));
    CHK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_mode_to_native(handle.wrap_modes(0))));
    CHK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_mode_to_native(handle.wrap_modes(1))));
    ++slot_;
}
