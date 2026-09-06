#include "Shader_Version_3_0.hpp"
#include <stdexcept>
#include <unordered_map>

using namespace Mlib;
using namespace std::string_view_literals;

#if defined (__ANDROID__)
static ShaderPlatform platform_ = ShaderPlatform::ANDROID;
#elif defined (__EMSCRIPTEN__)
static ShaderPlatform platform_ = ShaderPlatform::EMSCRIPTEN;
#else
static ShaderPlatform platform_ = ShaderPlatform::DESKTOP;
#endif

ShaderPlatform Mlib::shader_platform_from_string(std::string_view s) {
    static const std::unordered_map<std::string_view, ShaderPlatform> m{
        {"android"sv, ShaderPlatform::ANDROID},
        {"emscripten"sv, ShaderPlatform::EMSCRIPTEN},
        {"desktop"sv, ShaderPlatform::DESKTOP},
    };
    auto it = m.find(s);
    if (it == m.end()) {
        throw std::runtime_error("Unknown shader platform: " + std::string(s));
    }
    return it->second;
}

void Mlib::set_shader_platform(ShaderPlatform platform) {
    platform_ = platform;
}

std::string Mlib::vertex_shader_preamble() {
    switch (platform_) {
    case ShaderPlatform::ANDROID:
        return "#version 300 es\n";
    case ShaderPlatform::EMSCRIPTEN:
        return "#version 300 es\n";
    case ShaderPlatform::DESKTOP:
        return "#version 460 core\n";
    }
    throw std::runtime_error("Unknown shader platform: " + std::to_string((int)platform_));
}

std::string Mlib::fragment_shader_preamble() {
    switch (platform_) {
    case ShaderPlatform::ANDROID:
        return "#version 300 es\n"
               "precision mediump float;\nprecision mediump int;\n";
    case ShaderPlatform::EMSCRIPTEN:
        return "#version 300 es\n"
               "precision mediump float;\nprecision mediump int;\n";
    case ShaderPlatform::DESKTOP:
        return "#version 460 core\n";
    }
    throw std::runtime_error("Unknown shader platform: " + std::to_string((int)platform_));
}
