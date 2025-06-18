#pragma once
#include <Mlib/Render/Any_Gl.hpp>

namespace Mlib {

class ITextureHandle;

class TextureBinder {
public:
    explicit TextureBinder(GLint first_slot = 0);
    void bind(GLint uniform, const ITextureHandle& handle);
private:
    GLint slot_;
};

}
