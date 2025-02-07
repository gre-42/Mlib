#include "Buffer_Background_Copy.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Deallocation_Mode.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Threads/Launch_Async.hpp>

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
    }
    THROW_OR_ABORT("Unknown background copy state");
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

BufferBackgroundCopy::BufferBackgroundCopy(size_t min_bytes)
    : min_bytes_{ min_bytes }
    , buffer_{ (GLuint)-1 }
    , is_mapped_{ false }
    , state_{ BackgroundCopyState::UNINITIALIZED }
    , forked_{ false }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(DeallocationMode::DIRECT); }) }
{}

void BufferBackgroundCopy::set_type_erased(
    const char* begin,
    const char* end,
    TaskLocation task_location)
{
    if (forked_) {
        THROW_OR_ABORT("BufferBackgroundCopy::set_type_erased on forked object");
    }
    if (state_ != BackgroundCopyState::UNINITIALIZED) {
        THROW_OR_ABORT("Buffer already set, state: " + std::to_string((int)state_));
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
    if ((begin == nullptr) ||
        (integral_cast<size_t>(end - begin) < min_bytes_) ||
        (task_location == TaskLocation::FOREGROUND) ||
        !buffer_data_supported())
    {
        CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), begin, GL_STATIC_DRAW));
        is_mapped_ = false;
        state_ = BackgroundCopyState::AWAITED;
    } else {
#ifdef __ANDROID__
        verbose_abort("Internal error: buffer-data not supported on Android");
#else
        CHK(glBufferStorage(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT));
        CHK(char* dest = (char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        is_mapped_ = true;
        static LaunchAsync launch_async{ "Buffer BG copy" };
        future_ = launch_async([dest, begin, end]() {
            std::copy(begin, end, dest);
            });
#endif
        state_ = BackgroundCopyState::COPY_IN_PROGRESS;
    }
}

BufferBackgroundCopy::~BufferBackgroundCopy() {
    if (ContextQuery::is_initialized()) {
        deallocate(DeallocationMode::DIRECT);
    } else {
        deallocate(DeallocationMode::GARBAGE_COLLECTION);
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
        is_mapped_ = false;
    }
}

void BufferBackgroundCopy::update() {
    // Do nothing
}

void BufferBackgroundCopy::bind() const {
    CHK(glBindBuffer(GL_ARRAY_BUFFER, handle())); 
}

bool BufferBackgroundCopy::is_awaited() const {
    return state_ == BackgroundCopyState::AWAITED;
}

std::shared_ptr<IArrayBuffer> BufferBackgroundCopy::fork() {
    if ((state_ != BackgroundCopyState::AWAITED) &&
        (state_ != BackgroundCopyState::UNINITIALIZED))
    {
        // Objects are uninitialized after "BufferBackgroundCopy::deallocate".
        THROW_OR_ABORT(
            "BufferBackgroundCopy: attempt to fork an object that is neither "
            "awaited nor uninitialized. State: " + std::to_string((int)state_));
    }
    return shared_from_this();
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

void BufferBackgroundCopy::deallocate(DeallocationMode mode) {
    if (state_ == BackgroundCopyState::COPY_IN_PROGRESS) {
        future_.get();
    }
    if (state_ >= BackgroundCopyState::BUFFER_CREATED) {
        if (mode == DeallocationMode::DIRECT) {
            ABORT(glDeleteBuffers(1, &buffer_));
        } else if (mode == DeallocationMode::GARBAGE_COLLECTION) {
            render_gc_append_to_buffers(buffer_);
        } else {
            verbose_abort("Unknown deallocation mode");
        }
        buffer_ = (GLuint)-1;
    }
    state_ = BackgroundCopyState::UNINITIALIZED;
}
