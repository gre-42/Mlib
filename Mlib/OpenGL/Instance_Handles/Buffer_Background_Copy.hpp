#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/OpenGL/Any_Gl.hpp>
#include <Mlib/OpenGL/Instance_Handles/IArray_Buffer.hpp>
#include <Mlib/Scene_Graph/Render/Batch_Renderers/Task_Location.hpp>
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
    BUFFER_CREATED,
    COPY_IN_PROGRESS,
    READY,
    AWAITED
};

enum class BufferLifetime {
    RUNNING,
    SHUTTING_DOWN,
    DESTROYED
};

std::string background_copy_state_to_string(BackgroundCopyState s);

class BufferGenericCopy: public IArrayBuffer, public std::enable_shared_from_this<BufferGenericCopy> {
    BufferGenericCopy(const BufferGenericCopy &) = delete;
    BufferGenericCopy &operator=(const BufferGenericCopy &) = delete;

public:
    explicit BufferGenericCopy(
        TaskLocation task_location,
        size_t min_bytes = 1000);
    ~BufferGenericCopy();

    virtual bool copy_in_progress() const override;
    virtual void wait() const override;
    virtual void update() override;
    virtual void bind() const override;
    virtual bool is_initialized() const override;
    virtual std::shared_ptr<IArrayBuffer> fork() override;

    GLuint handle() const;
    inline BackgroundCopyState state() const {
        return state_;
    }

protected:
    virtual void init_type_erased(
        const std::byte* begin,
        const std::byte* end) override;
    virtual void substitute_type_erased(
        const std::byte* begin,
        const std::byte* end) override;
private:
    void deallocate(DeallocationMode mode);
    BufferLifetime buffer_lifetime_;
    TaskLocation task_location_;
    GLsizeiptr min_bytes_;
    GLuint buffer_;
    GLsizeiptr capacity_;
    mutable std::future<void> future_;
    mutable std::byte* memory_map_;
    mutable BackgroundCopyState state_;
    bool forked_;
    std::thread::id render_thread_id_;
    DeallocationToken deallocation_token_;
};

class BufferForegroundCopy: public BufferGenericCopy {
public:
    explicit BufferForegroundCopy();
    ~BufferForegroundCopy();
};

class BufferBackgroundCopy: public BufferGenericCopy {
public:
    explicit BufferBackgroundCopy(size_t min_bytes = 1000);
    ~BufferBackgroundCopy();
};

}
