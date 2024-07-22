#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <Mlib/Render/Instance_Handles/IArray_Buffer.hpp>
#include <future>
#include <memory>
#include <string>
#include <thread>
#include <type_traits>
#include <vector>

namespace Mlib {

enum class DeallocationMode;

enum class BackgroundCopyState {
    UNINITIALIZED,
    UNUSED,
    BUFFER_CREATED,
    COPY_IN_PROGRESS,
    READY,
    AWAITED
};

std::string background_copy_state_to_string(BackgroundCopyState s);

class BufferBackgroundCopy: public IArrayBuffer, public std::enable_shared_from_this<BufferBackgroundCopy> {
    BufferBackgroundCopy(const BufferBackgroundCopy &) = delete;
    BufferBackgroundCopy &operator=(const BufferBackgroundCopy &) = delete;

public:
    explicit BufferBackgroundCopy(size_t min_bytes = 1000);
    ~BufferBackgroundCopy();

    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind() const override;
    virtual bool is_awaited() const override;
    virtual std::shared_ptr<IArrayBuffer> fork() override;

    GLuint handle() const;
    inline BackgroundCopyState state() const {
        return state_;
    }

protected:
    virtual void set_type_erased(
        const char *begin,
        const char *end,
        TaskLocation task_location) override;

private:
    void deallocate(DeallocationMode mode);
    size_t min_bytes_;
    GLuint buffer_;
    mutable std::future<void> future_;
    mutable bool is_mapped_;
    mutable BackgroundCopyState state_;
    bool forked_;
    std::thread::id render_thread_id_;
    DeallocationToken deallocation_token_;
};

}
