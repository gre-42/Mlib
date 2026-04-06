
#include "Print_Gl_Version_Info.hpp"
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void Mlib::print_gl_version_info(std::ostream& ostr) {
    const char* vendor = (const char*)CHK_X(glGetString(GL_VENDOR));
    const char* renderer = (const char*)CHK_X(glGetString(GL_RENDERER));
    const char* shading_language_version = (const char*)CHK_X(glGetString(GL_SHADING_LANGUAGE_VERSION));
    ostr << "OpenGL information\n";
    ostr << "  - Vendor: " << vendor << '\n';
    ostr << "  - Renderer: " << renderer << '\n';
    ostr << "  - Shading language version: " << shading_language_version << '\n';
}
