#pragma once
#include <string>
#include <string_view>

#undef EMSCRIPTEN

enum class ShaderPlatform {
    ANDROID,
    EMSCRIPTEN,
    DESKTOP
};

namespace Mlib {

ShaderPlatform shader_platform_from_string(std::string_view s);
void set_shader_platform(ShaderPlatform platform);

std::string vertex_shader_preamble();
std::string fragment_shader_preamble();

}
