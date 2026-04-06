#include "Buffer_Background_Copy.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/OpenGL/CHK.hpp>
#include <Mlib/OpenGL/Context_Query.hpp>
#include <Mlib/OpenGL/Deallocate/Deallocation_Mode.hpp>
#include <Mlib/OpenGL/Deallocate/Render_Deallocator.hpp>
#include <Mlib/OpenGL/Deallocate/Render_Garbage_Collector.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Launch_Async.hpp>

using namespace Mlib;

std::string Mlib::background_copy_state_to_string(BackgroundCopyState s) {
    switch (s) {
    case BackgroundCopyState::UNINITIALIZED:
        return "uninitialized";
    case BackgroundCopyState::BUFFER_CREATED:
        return "buffer_created";
    case BackgroundCopyState::COPY_IN_PROGRESS:
        return "copy_in_progress";
    case BackgroundCopyState::READY:
        return "ready";
    case BackgroundCopyState::AWAITED:
        return "awaited";
    }
    throw std::runtime_error("Unknown background copy state");
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

BufferGenericCopy::BufferGenericCopy(
    TaskLocation task_location,
    size_t min_bytes)
    : buffer_lifetime_{BufferLifetime::RUNNING}
    , task_location_{ task_location }
    , min_bytes_{ integral_cast<GLsizeiptr>(min_bytes) }
    , buffer_{ (GLuint)-1 }
    , capacity_{ -1 }
    , memory_map_{ nullptr }
    , state_{ BackgroundCopyState::UNINITIALIZED }
    , forked_{ false }
    , deallocation_token_{ render_deallocator.insert([this]() { deallocate(DeallocationMode::DIRECT); }) }
{}

void BufferGenericCopy::init_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::init_type_erased on a non-running object");
    }
    if (forked_) {
        throw std::runtime_error("BufferGenericCopy::init_type_erased on forked object");
    }
    if (state_ != BackgroundCopyState::UNINITIALIZED) {
        throw std::runtime_error("Buffer already set, state: " + background_copy_state_to_string(state_));
    }
    if (buffer_ != (GLuint)-1) {
        verbose_abort("Buffer already set (2)");
    }
    render_thread_id_ = std::this_thread::get_id();
    CHK(glGenBuffers(1, &buffer_));
    state_ = BackgroundCopyState::BUFFER_CREATED;
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));

    auto size = integral_cast<GLsizeiptr>(end - begin);
    if ((begin == nullptr) && (end == nullptr)) {
        throw std::runtime_error("BufferGenericCopy::init_type_erased received zero capacity");
    }
    if (begin == nullptr) {
        capacity_ = size;
    }
    if ((size < min_bytes_) ||
        (task_location_ == TaskLocation::FOREGROUND) ||
        !buffer_data_supported())
    {
        CHK(glBufferData(GL_ARRAY_BUFFER, size, begin, ((capacity_ != -1) ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW)));
        memory_map_ = nullptr;
        state_ = BackgroundCopyState::AWAITED;
    } else {
#ifdef __ANDROID__
        verbose_abort("Internal error: buffer-data not supported on Android");
#else
        CHK(glBufferStorage(GL_ARRAY_BUFFER, size, nullptr,
            GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | ((capacity_ != -1) ? GL_DYNAMIC_STORAGE_BIT : 0)));
        if (capacity_ != -1) {
            memory_map_ = nullptr;
            state_ = BackgroundCopyState::AWAITED;
        } else {
            memory_map_ = (std::byte*)CHK_X(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
            static LaunchAsync launch_async{ "Buffer BG copy (init)" };
            future_ = launch_async([this, begin, end]() {
                std::copy(begin, end, memory_map_);
                });
            state_ = BackgroundCopyState::COPY_IN_PROGRESS;
        }
#endif
    }
}

void BufferGenericCopy::substitute_type_erased(
    const std::byte* begin,
    const std::byte* end)
{
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::substitute_type_erased on a non-running object");
    }
    if (forked_) {
        throw std::runtime_error("BufferGenericCopy::substitute_type_erased on forked object");
    }
    if (capacity_ == -1) {
        throw std::runtime_error("BufferGenericCopy::substitute_type_erased on a static object");
    }
    [&](){
        switch (state_) {
        case BackgroundCopyState::UNINITIALIZED:
        case BackgroundCopyState::BUFFER_CREATED:
            throw std::runtime_error(
                "BufferGenericCopy::substitute_type_erased called on unexpected state (0): " +
                background_copy_state_to_string(state_));
        case BackgroundCopyState::AWAITED:
        case BackgroundCopyState::COPY_IN_PROGRESS:
        case BackgroundCopyState::READY:
            return;
        }
        throw std::runtime_error(
            "BufferGenericCopy::substitute_type_erased called on unexpected state (1): " +
            background_copy_state_to_string(state_));
    }();
    if (std::this_thread::get_id() != render_thread_id_) {
        throw std::runtime_error("BufferGenericCopy::substitute_type_erased called from wrong or uninitialized thread");
    }
    if ((begin == nullptr) && (begin != end)) {
        throw std::runtime_error("BufferGenericCopy::substitute_type_erased received null begin");
    }
    auto size = integral_cast<GLsizeiptr>(end - begin);
    if (size > capacity_) {
        throw std::runtime_error(
            "BufferGenericCopy capacity: " + std::to_string(capacity_) +
            ", size: " + std::to_string(size));
    }
    bind();
    if ((size < min_bytes_) ||
        (task_location_ == TaskLocation::FOREGROUND) ||
        !buffer_data_supported())
    {
        CHK(glBufferSubData(GL_ARRAY_BUFFER, 0, size, begin));
        state_ = BackgroundCopyState::AWAITED;
    } else {
#ifdef __ANDROID__
        verbose_abort("Internal error: buffer-data not supported on Android");
#else
        if (memory_map_ != nullptr) {
            throw std::runtime_error("Buffer already mapped");
        }
        memory_map_ = (std::byte*)CHK_X(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        static LaunchAsync launch_async{ "Buffer BG copy (substitute)" };
        future_ = launch_async([this, begin, end]() {
            std::copy(begin, end, memory_map_);
            });
#endif
        state_ = BackgroundCopyState::COPY_IN_PROGRESS;
    }
}

BufferGenericCopy::~BufferGenericCopy() {
    buffer_lifetime_ = BufferLifetime::SHUTTING_DOWN;
    if (ContextQuery::is_initialized()) {
        deallocate(DeallocationMode::DIRECT);
    } else {
        deallocate(DeallocationMode::GARBAGE_COLLECTION);
    }
    buffer_lifetime_ = BufferLifetime::DESTROYED;
}

void BufferGenericCopy::wait() const {
    if (std::this_thread::get_id() != render_thread_id_) {
        throw std::runtime_error("BufferGenericCopy::wait called from the wrong or uninitialized thread");
    }
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::wait on a non-running object");
    }
    if (state_ == BackgroundCopyState::AWAITED) {
        return;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        return;
    }
    if ((state_ != BackgroundCopyState::COPY_IN_PROGRESS) &&
        (state_ != BackgroundCopyState::READY))
    {
        throw std::runtime_error("Waiting for an incomplete buffer");
    }
    future_.get();
    state_ = BackgroundCopyState::AWAITED;
    if (memory_map_ != nullptr) {
        CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
        CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
        memory_map_ = nullptr;
    }
}

void BufferGenericCopy::update() {
    // Do nothing
}

void BufferGenericCopy::bind() const {
    CHK(glBindBuffer(GL_ARRAY_BUFFER, handle())); 
}

bool BufferGenericCopy::is_initialized() const {
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::is_initialized on a non-running object");
    }
    return state_ != BackgroundCopyState::UNINITIALIZED;
}

std::shared_ptr<IArrayBuffer> BufferGenericCopy::fork() {
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::fork on a non-running object");
    }
    if ((state_ != BackgroundCopyState::AWAITED) &&
        (state_ != BackgroundCopyState::UNINITIALIZED))
    {
        // Objects are uninitialized after "BufferGenericCopy::deallocate".
        throw std::runtime_error(
            "BufferGenericCopy: attempt to fork an object that is neither "
            "awaited nor uninitialized. State: " + background_copy_state_to_string(state_));
    }
    return shared_from_this();
}

GLuint BufferGenericCopy::handle() const {
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

bool BufferGenericCopy::copy_in_progress() const {
    if (std::this_thread::get_id() != render_thread_id_) {
        throw std::runtime_error("BufferGenericCopy::copy_in_progress called from the wrong or uninitialized thread");
    }
    if (buffer_lifetime_ != BufferLifetime::RUNNING) {
        verbose_abort("BufferGenericCopy::copy_in_progress on a non-running object");
    }
    if (state_ == BackgroundCopyState::AWAITED) {
        return false;
    }
    if (state_ == BackgroundCopyState::READY) {
        return false;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        throw std::runtime_error("Checking copy_in_progress on uninitialized buffer");
    }
    if (state_ != BackgroundCopyState::COPY_IN_PROGRESS) {
        throw std::runtime_error("Checking copy_in_progress on broken buffer");
    }
    if (!future_.valid()) {
        verbose_abort("BufferGenericCopy::copy_in_progress future not valid");
    }
    if (is_ready(future_)) {
        state_ = BackgroundCopyState::READY;
        return false;
    }
    return true;
}

void BufferGenericCopy::deallocate(DeallocationMode mode) {
    if ((buffer_lifetime_ != BufferLifetime::SHUTTING_DOWN) &&
        (render_thread_id_ != std::thread::id()) &&
        (std::this_thread::get_id() != render_thread_id_))
    {
        throw std::runtime_error("BufferGenericCopy::deallocate called from the wrong thread");
    }
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

BufferForegroundCopy::BufferForegroundCopy()
    : BufferGenericCopy{TaskLocation::FOREGROUND, 1234}
{}

BufferForegroundCopy::~BufferForegroundCopy() = default;


BufferBackgroundCopy::BufferBackgroundCopy(size_t min_bytes)
    : BufferGenericCopy{TaskLocation::BACKGROUND, min_bytes}
{}

BufferBackgroundCopy::~BufferBackgroundCopy() = default;
