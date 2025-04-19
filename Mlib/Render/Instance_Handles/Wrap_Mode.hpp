#pragma once
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <string>

namespace Mlib {

inline GLint wrap_mode_to_native(WrapMode mode) {
    switch(mode) {
    case WrapMode::REPEAT:
        return GL_REPEAT;
    case WrapMode::CLAMP_TO_EDGE:
        return GL_CLAMP_TO_EDGE;
    case WrapMode::CLAMP_TO_BORDER:
        return GL_CLAMP_TO_BORDER;
    }
    THROW_OR_ABORT("Unknown wrap mode: " + std::to_string((int)mode));
}

inline WrapMode wrap_mode_from_native(GLint mode) {
    switch (mode) {
	case GL_REPEAT:
        return WrapMode::REPEAT;
	case GL_CLAMP_TO_EDGE:
        return WrapMode::CLAMP_TO_EDGE;
	case GL_CLAMP_TO_BORDER:
        return WrapMode::CLAMP_TO_BORDER;
	default:
        THROW_OR_ABORT("Unsupported wrap mode: " + std::to_string(mode));
    }
}

}
