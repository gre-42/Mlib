#include "Render_Guards.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Instance_Handles/IFrame_Buffer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <iostream>

using namespace Mlib;

static std::shared_ptr<IFrameBuffer> last_frame_buffer = nullptr;
static std::shared_ptr<IFrameBuffer> bound_frame_buffer = nullptr;
static SourceLocation bound_source_location = CURRENT_SOURCE_LOCATION;

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

RenderToFrameBufferGuard::RenderToFrameBufferGuard(std::shared_ptr<IFrameBuffer> fb)
    : previous_frame_buffer_{ last_frame_buffer }
{
    if (!fb->is_configured()) {
        THROW_OR_ABORT("Frame buffer has not been configured");
    }
    last_frame_buffer = std::move(fb);
}

RenderToFrameBufferGuard::~RenderToFrameBufferGuard() {
    if (last_frame_buffer != bound_frame_buffer) {
        verbose_abort("~RenderToFrameBufferGuard error");
    }
    last_frame_buffer->unbind(CURRENT_SOURCE_LOCATION);
    bound_frame_buffer = nullptr;
    last_frame_buffer = previous_frame_buffer_;
}

void Mlib::notify_rendering(SourceLocation loc) {
    bool is_active = (bound_frame_buffer != last_frame_buffer);
    if (!is_active) {
        return;
    }
    if (last_frame_buffer != nullptr) {
        if (bound_frame_buffer != nullptr) {
            bound_frame_buffer->unbind(CURRENT_SOURCE_LOCATION);
        }
        last_frame_buffer->bind(loc);
        bound_frame_buffer = last_frame_buffer;
        bound_source_location = loc;
    }
}
