#include "Texture_Binder.hpp"
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TextureBinder::TextureBinder(GLint first_slot)
    : slot_{ first_slot }
{}

void TextureBinder::bind(
    GLint uniform,
    const ITextureHandle& handle,
    InterpolationPolicy interpolation_policy)
{
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
    auto w0 = handle.wrap_modes(0);
    auto w1 = handle.wrap_modes(1);
    CHK(glTexParameteri(target, GL_TEXTURE_WRAP_S, wrap_mode_to_native(w0)));
    CHK(glTexParameteri(target, GL_TEXTURE_WRAP_T, wrap_mode_to_native(w1)));
    if ((w0 == WrapMode::CLAMP_TO_BORDER) || (w1 == WrapMode::CLAMP_TO_BORDER)) {
        CHK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, handle.border_color().flat_begin()));
    }
    if ((interpolation_policy == InterpolationPolicy::AUTO) &&
        (handle.magnifying_interpolation_mode() == InterpolationMode::LINEAR))
    {
        if (handle.mipmap_mode() == MipmapMode::NO_MIPMAPS) {
            CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        } else {
            CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        }
    } else {
        CHK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
    ++slot_;
}
