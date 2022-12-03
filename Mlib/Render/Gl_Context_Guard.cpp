#include "Gl_Context_Guard.hpp"
#include <Mlib/Render/IWindow.hpp>
#include <Mlib/Render/Render_Garbage_Collector.hpp>

using namespace Mlib;

GlContextGuard::GlContextGuard(const IWindow& window)
: window_{window}
{
    window_.make_current();
    execute_render_gc();
}

GlContextGuard::~GlContextGuard() {
    execute_render_gc();
    window_.unmake_current();
}
