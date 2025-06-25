#include "Texture_Binder.hpp"
#include <Mlib/Geometry/Material/Interpolation_Mode.hpp>
#include <Mlib/Geometry/Material/Mipmap_Mode.hpp>
#include <Mlib/Geometry/Material/Texture_Target.hpp>
#include <Mlib/Geometry/Texture/ITexture_Handle.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/Texture_Layer_Properties.hpp>
#include <Mlib/Render/Instance_Handles/Wrap_Mode.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

TextureBinder::TextureBinder(GLint first_slot)
    : slot_{ first_slot }
{}

TextureBinder::~TextureBinder() {
    ABORT(glActiveTexture(GL_TEXTURE0));
}

void TextureBinder::bind(
    GLint uniform,
    const ITextureHandle& handle,
    InterpolationPolicy interpolation_policy,
    TextureLayerProperties layer_properties)
{
    if (uniform < 0) {
        THROW_OR_ABORT("Uniform ID is negative");
    }
    auto target = handle.target();
    if (any(target & TextureTarget::ONE_LAYER_MASK) && (handle.layers() != 1)) {
        THROW_OR_ABORT("2D texture does not have exactly one layer, but " + std::to_string(handle.layers()));
    }
    if (!any(target & TextureTarget::ONE_LAYER_MASK) && (handle.layers() < 2)) {
        THROW_OR_ABORT("3D texture or texture array has less than two layers, but " + std::to_string(handle.layers()));
    }
    auto cont = any(layer_properties & TextureLayerProperties::CONTINUOUS);
    auto disc = any(layer_properties & TextureLayerProperties::DISCRETE);
    auto mip2 = (handle.mipmap_mode() == MipmapMode::WITH_MIPMAPS_2D);
    auto target2 = (cont && !mip2)
        ? TextureTarget::TEXTURE_3D
        : (disc || mip2)
            ? TextureTarget::TEXTURE_2D_ARRAY
            : TextureTarget::ONE_LAYER_MASK;
    if (target2 == TextureTarget::ONE_LAYER_MASK) {
        if (!any(target & TextureTarget::ONE_LAYER_MASK)) {
            THROW_OR_ABORT("Unexpected texture target (0)");
        }
    } else if (target != target2) {
        THROW_OR_ABORT("Unexpected texture target (1)");
    }
    auto target_native = [&](){
        switch (target) {
        case TextureTarget::NONE: THROW_OR_ABORT("Texture binder received \"none\" target");
        case TextureTarget::TEXTURE_2D: return GL_TEXTURE_2D;
        case TextureTarget::TEXTURE_2D_ARRAY: return GL_TEXTURE_2D_ARRAY;
        case TextureTarget::TEXTURE_3D: return GL_TEXTURE_3D;
        case TextureTarget::TEXTURE_CUBE_MAP: return GL_TEXTURE_CUBE_MAP;
        case TextureTarget::ONE_LAYER_MASK: THROW_OR_ABORT("Texture binder received \"one_layer_mask\" target");
        }
        THROW_OR_ABORT("Unknown texture target");
    }();
    CHK(glUniform1i(uniform, slot_));
    CHK(glActiveTexture(GL_TEXTURE0 + slot_));
    CHK(glBindTexture(target_native, handle.handle<GLuint>()));
    auto w0 = handle.wrap_modes(0);
    auto w1 = handle.wrap_modes(1);
    CHK(glTexParameteri(target_native, GL_TEXTURE_WRAP_S, wrap_mode_to_native(w0)));
    CHK(glTexParameteri(target_native, GL_TEXTURE_WRAP_T, wrap_mode_to_native(w1)));
    if ((w0 == WrapMode::CLAMP_TO_BORDER) || (w1 == WrapMode::CLAMP_TO_BORDER)) {
        CHK(glTexParameterfv(target_native, GL_TEXTURE_BORDER_COLOR, handle.border_color().flat_begin()));
    }
    if ((interpolation_policy == InterpolationPolicy::AUTO) &&
        (handle.magnifying_interpolation_mode() == InterpolationMode::LINEAR))
    {
        if (handle.mipmap_mode() == MipmapMode::NO_MIPMAPS) {
            CHK(glTexParameteri(target_native, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
            CHK(glTexParameteri(target_native, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        } else {
            CHK(glTexParameteri(target_native, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
            CHK(glTexParameteri(target_native, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
        }
    } else {
        CHK(glTexParameteri(target_native, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
        CHK(glTexParameteri(target_native, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
    }
    ++slot_;
}
