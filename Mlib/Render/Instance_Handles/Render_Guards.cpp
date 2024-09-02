#include "Render_Guards.hpp"
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

static IFrameBuffer* last_frame_buffer = nullptr;
static IFrameBuffer* bound_frame_buffer = nullptr;

// use cases:
// 1.
// {
//     RenderToScreenGuard rsg;
//     render();
// }
// 2.
// {
//     RenderToFrameBufferGuard rfg;
//     RenderToScreenGuard rsg;
//     render();
// }
// 3.
// {
//     RenderToFrameBufferGuard rfg0;
//     {
//         RenderToFrameBufferGuard rfg1;
//         RenderToScreenGuard rsg;
//         render();
//     }
//     {
//         RenderToScreenGuard rsg;
//         render();
//     }
//     glReadPixels();
// }

RenderToFrameBufferGuard::RenderToFrameBufferGuard(IFrameBuffer& fb)
    : previous_frame_buffer_{ last_frame_buffer }
{
    if (!fb.is_configured()) {
        THROW_OR_ABORT("Frame buffer has not been configured");
    }
    last_frame_buffer = &fb;
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    last_frame_buffer = previous_frame_buffer_;
}

RenderToScreenGuard::RenderToScreenGuard(SourceLocation loc) {
    is_active_ = (bound_frame_buffer != last_frame_buffer);
    if (!is_active_) {
        return;
    }
    if (last_frame_buffer != nullptr) {
        if (bound_frame_buffer != nullptr) {
            THROW_OR_ABORT("Another frame buffer is already bound");
        }
        last_frame_buffer->bind(loc);
        bound_frame_buffer = last_frame_buffer;
    }
}

RenderToScreenGuard::~RenderToScreenGuard() {
    if (!is_active_) {
        return;
    }
    if (last_frame_buffer != nullptr) {
        last_frame_buffer->unbind(CURRENT_SOURCE_LOCATION);
        bound_frame_buffer = nullptr;
    }
}
