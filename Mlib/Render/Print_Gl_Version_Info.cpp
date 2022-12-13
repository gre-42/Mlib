#include "Print_Gl_Version_Info.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void Mlib::print_gl_version_info() {
    const char* vendor = CHK((const char*)glGetString(GL_VENDOR));
    const char* renderer = CHK((const char*)glGetString(GL_RENDERER));
    const char* shading_language_version = CHK((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    linfo() << "Vendor: " << vendor;
    linfo() << "Renderer: " << renderer;
    linfo() << "Shading language version: " << shading_language_version;
}
