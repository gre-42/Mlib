#pragma once
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class ITextureHandle;

enum class InterpolationPolicy {
    AUTO,
    NEAREST_NEIGHBOR
};

class TextureBinder {
public:
    explicit TextureBinder(GLint first_slot = 0);
    void bind(
        GLint uniform,
        const ITextureHandle& handle,
        InterpolationPolicy interpolation_policy);
private:
    GLint slot_;
};

}
