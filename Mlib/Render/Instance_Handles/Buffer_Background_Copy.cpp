#include "Buffer_Background_Copy.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Render/CHK.hpp>
#include <Mlib/Render/Context_Query.hpp>
#include <Mlib/Render/Deallocate/Render_Deallocator.hpp>
#include <Mlib/Render/Deallocate/Render_Garbage_Collector.hpp>

using namespace Mlib;

BufferBackgroundCopy::BufferBackgroundCopy()
: is_mapped_{false},
  state_{BackgroundCopyState::UNINITIALIZED},
  deallocation_token_{render_deallocator.insert([this](){deallocate();})}
{}

void BufferBackgroundCopy::set_type_erased(const char* begin, const char* end)
{
    if (state_ != BackgroundCopyState::UNINITIALIZED) {
        THROW_OR_ABORT("Buffer already set");
    }
    CHK(glGenBuffers(1, &buffer_));
    state_ = BackgroundCopyState::BUFFER_CREATED;
    CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));

    // Do not unbind so that attributes can be set.
    // CHK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    if (begin == nullptr) {
        CHK(glBufferData(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), nullptr, GL_STATIC_DRAW));
        is_mapped_ = false;
        std::promise<void> p;
        p.set_value();
        future_ = p.get_future();
    } else {
        CHK(glBufferStorage(GL_ARRAY_BUFFER, integral_cast<GLsizeiptr>(end - begin), nullptr, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT));
        CHK(char* dest = (char*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
        is_mapped_ = true;
        future_ = std::async(std::launch::async, [dest, begin, end](){
            std::copy(begin, end, dest);
        });
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
    if (state_ == BackgroundCopyState::AWAITED) {
        return;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        return;
    }
    if (state_ != BackgroundCopyState::COPY_IN_PROGRESS) {
        THROW_OR_ABORT("Waiting for an incomplete buffer");
    }
    future_.get();
    state_ = BackgroundCopyState::AWAITED;
    if (is_mapped_) {
        CHK(glBindBuffer(GL_ARRAY_BUFFER, buffer_));
        CHK(glUnmapBuffer(GL_ARRAY_BUFFER));
    }
}

// From: https://stackoverflow.com/questions/10890242/get-the-status-of-a-stdfuture
template<typename R>
static bool is_ready(std::future<R> const& f) {
    return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}

bool BufferBackgroundCopy::copy_in_progress() const {
    if (state_ == BackgroundCopyState::AWAITED) {
        return false;
    }
    if (state_ == BackgroundCopyState::UNINITIALIZED) {
        return false;
    }
    if (state_ != BackgroundCopyState::COPY_IN_PROGRESS) {
        THROW_OR_ABORT("Checking copy_in_progress on broken buffer");
    }
    return !is_ready(future_);
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
