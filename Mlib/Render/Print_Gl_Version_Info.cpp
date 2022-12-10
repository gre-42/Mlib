#include "Print_Gl_Version_Info.hpp"
#include <Mlib/Render/CHK.hpp>

#ifdef __ANDROID__
#include <NDKHelper.h>

using namespace Mlib;

void Mlib::print_gl_version_info() {
    const char* vendor = CHK((const char*)glGetString(GL_VENDOR));
    const char* renderer = CHK((const char*)glGetString(GL_RENDERER));
    const char* shading_language_version = CHK((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOGI("Vendor: %s", vendor);
    LOGI("Renderer: %s", renderer);
    LOGI("Shading language version: %s", shading_language_version);
}
#else
#include <iostream>

using namespace Mlib;

void Mlib::print_gl_version_info() {
    const char* vendor = CHK((const char*)glGetString(GL_VENDOR));
    const char* renderer = CHK((const char*)glGetString(GL_RENDERER));
    const char* shading_language_version = CHK((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    std::cerr << "Vendor: " << vendor << std::endl;
    std::cerr << "Renderer: " << renderer << std::endl;
    std::cerr << "Shading language version: " << shading_language_version << std::endl;
}

#endif
