#include "Print_Gl_Version_Info.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>

using namespace Mlib;

void Mlib::print_gl_version_info() {
    CHK(const char* vendor = (const char*)glGetString(GL_VENDOR));
    CHK(const char* renderer = (const char*)glGetString(GL_RENDERER));
    CHK(const char* shading_language_version = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
    linfo() << "Vendor: " << vendor;
    linfo() << "Renderer: " << renderer;
    linfo() << "Shading language version: " << shading_language_version;
}
