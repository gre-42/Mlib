#pragma once
#include <Mlib/Geometry/Material/Wrap_Mode.hpp>
#include <Mlib/OpenGL/Any_Gl.hpp>
#include <stdexcept>
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
    throw std::runtime_error("Unknown wrap mode: " + std::to_string((int)mode));
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
        throw std::runtime_error("Unsupported wrap mode: " + std::to_string(mode));
    }
}

}
