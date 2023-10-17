#include "Buffer_Background_Copy.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

std::string Mlib::background_copy_state_to_string(BackgroundCopyState s) {
    switch (s) {
    case BackgroundCopyState::UNINITIALIZED:
        return "uninitialized";
    case BackgroundCopyState::UNUSED:
        return "unused";
    case BackgroundCopyState::BUFFER_CREATED:
        return "buffer_created";
    case BackgroundCopyState::COPY_IN_PROGRESS:
        return "copy_in_progress";
    case BackgroundCopyState::READY:
        return "ready";
    case BackgroundCopyState::AWAITED:
        return "awaited";
    default:
        THROW_OR_ABORT("Unknown background copy state");
    }
}

static bool buffer_data_supported() {
#ifdef __ANDROID__
	return false;
#else
	GLint major, minor;
	CHK(glGetIntegerv(GL_MAJOR_VERSION, &major));
	CHK(glGetIntegerv(GL_MINOR_VERSION, &minor));
	return std::make_pair(major, minor) >= std::make_pair(4, 4);
#endif
}

BufferBackgroundCopy::BufferBackgroundCopy()
    : buffer_{(GLuint)-1}
    , is_mapped_{false}
    , state_{BackgroundCopyState::UNINITIALIZED}
    , deallocation_token_{render_deallocator.insert([this]() { deallocate(); })} {
}

void BufferBackgroundCopy::set_type_erased(const char* begin, const char* end)
{
    if (state_ != BackgroundCopyState::UNINITIALIZED) {
        THROW_OR_ABORT("Buffer already set");
    }
    if (buffer_ != (GLuint)-1) {
        verbose_abort("Buffer already set (2)");
    }
    render_thread_id_ = std::this_thread::get_id();
    CHK(glGenBuffers(1, &buffer_));
    state_ = BackgroundCopyState::BUFFER_CREATED;
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));

    // Do not unbind so that attributes can be set.
    // CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	if ((begin == nullptr) || !buffer_data_supported()) {
		CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), begin, GL_STATIC_DRAW));
		is_mapped_ = false;
		std::promise<void> p;
		p.set_value();
		future_ = p.get_future();
	} else {
#ifdef __ANDROID__
        verbose_abort("internal error: buffer_data_supported not supported on Android");
#else
        CHK(glBufferStorage(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT));
        CHK(char* dest = (char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        is_mapped_ = true;
        future_ = std::async(std::launch::async, [dest, begin, end]() {
            std::copy(begin, end, dest);
        });
#endif
	}
    state_ = BackgroundCopyState::COPY_IN_PROGRESS;
}

BufferBackgroundCopy::~BufferBackgroundCopy() {
    if (state_ == BackgroundCopyState::COPY_IN_PROGRESS) {
        future_.get();
        state_ = BackgroundCopyState::AWAITED;
    }
    if (ContextQuery::is_initialized()) {
        deallocate();
    } else {
        gc_deallocate();
    }
}

void BufferBackgroundCopy::wait() const {
    if ((render_thread_id_ != std::thread::id()) &&
        (std::this_thread::get_id() != render_thread_id_))
    {
        THROW_OR_ABORT("BufferBackgroundCopy::wait called from the wrong thread");
    }
    if (state_ == BackgroundCopyState::AWAITED) {
        return;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        return;
    }
    if (state_ == BackgroundCopyState::UNUSED) {
        return;
    }
    if ((state_ != BackgroundCopyState::COPY_IN_PROGRESS) &&
        (state_ != BackgroundCopyState::READY))
    {
        THROW_OR_ABORT("Waiting for an incomplete buffer");
    }
    future_.get();
    state_ = BackgroundCopyState::AWAITED;
    if (is_mapped_) {
        CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
        CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    }
}

GLuint BufferBackgroundCopy::handle() const {
    wait();
    if (state_ != BackgroundCopyState::AWAITED) {
        verbose_abort("Invalid buffer handle");
    }
    return buffer_;
}

// From: https://stackoverflow.com/questions/10890242/get-the-status-of-a-stdfuture
template<typename R>
static bool is_ready(std::future<R> const& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool BufferBackgroundCopy::copy_in_progress() const {
    if ((render_thread_id_ != std::thread::id()) &&
        (std::this_thread::get_id() != render_thread_id_))
    {
        THROW_OR_ABORT("BufferBackgroundCopy::copy_in_progress called from the wrong thread");
    }
    if (state_ == BackgroundCopyState::AWAITED) {
        return false;
    }
    if (state_ == BackgroundCopyState::READY) {
        return false;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        state_ = BackgroundCopyState::UNUSED;
        return false;
    }
    if (state_ == BackgroundCopyState::UNUSED) {
        return false;
    }
    if (state_ != BackgroundCopyState::COPY_IN_PROGRESS) {
        THROW_OR_ABORT("Checking copy_in_progress on broken buffer");
    }
    if (!future_.valid()) {
        verbose_abort("BufferBackgroundCopy::copy_in_progress future not valid");
    }
    if (is_ready(future_)) {
        state_ = BackgroundCopyState::READY;
        return false;
    }
    return true;
}

void BufferBackgroundCopy::deallocate() {
    if (state_ >= BackgroundCopyState::BUFFER_CREATED) {
        ABORT(glDeleteBuffers(1, &buffer_));
    }
}

void BufferBackgroundCopy::gc_deallocate() {
    if (state_ >= BackgroundCopyState::BUFFER_CREATED) {
        render_gc_append_to_buffers(buffer_);
    }
}
