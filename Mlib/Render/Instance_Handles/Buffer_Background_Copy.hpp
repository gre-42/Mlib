#pragma once
#include <Mlib/Memory/Deallocation_Token.hpp>
#include <Mlib/Render/Any_Gl.hpp>
#include <future>
#include <thread>
#include <type_traits>
#include <vector>

namespace Mlib {

enum class BackgroundCopyState {
    UNINITIALIZED,
    UNUSED,
    BUFFER_CREATED,
    COPY_IN_PROGRESS,
    AWAITED
};

class BufferBackgroundCopy {
    BufferBackgroundCopy(const BufferBackgroundCopy&) = delete;
    BufferBackgroundCopy& operator = (const BufferBackgroundCopy&) = delete;
public:
    BufferBackgroundCopy();
    template <class TData>
    void reserve(size_t size) {
        set((TData*)nullptr, (TData*)nullptr + size);
    }
    template <class TData>
    void set(const std::vector<TData>& vec) {
        set(vec.begin(), vec.end());
    }
    template <class TIter>
    requires std::is_trivially_copyable_v<std::remove_reference_t<decltype(*TIter())>>
    void set(const TIter& begin, const TIter& end) {
        set_type_erased(
            reinterpret_cast<const char*>(&*begin),
            reinterpret_cast<const char*>(&*end));
    }
    template <typename T, size_t size>
    void set(const T (&array)[size]) {
        set(array + 0, array + size);
    }
    ~BufferBackgroundCopy();
    void deallocate();
    void gc_deallocate();
    void wait() const;
    GLuint handle() const;
    inline bool is_good() const {
        return (state_ >= BackgroundCopyState::COPY_IN_PROGRESS);
    }
    bool copy_in_progress() const;
private:
    void set_type_erased(const char* begin, const char* end);
    GLuint buffer_;
    mutable std::future<void> future_;
    bool is_mapped_;
    mutable BackgroundCopyState state_;
    std::thread::id render_thread_id_;
    DeallocationToken deallocation_token_;
};

}
