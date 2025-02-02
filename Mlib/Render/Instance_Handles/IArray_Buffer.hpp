#pragma once
#include <Mlib/Scene_Graph/Batch_Renderers/Task_Location.hpp>
#include <cstddef>
#include <memory>
#include <vector>

namespace Mlib {

class IArrayBuffer {
public:
    template <class TData>
    void reserve(size_t size) {
        set((TData *)nullptr, (TData *)nullptr + size, TaskLocation::FOREGROUND);
    }
    template <class TData>
    void set(const std::vector<TData> &vec, TaskLocation task_location) {
        set(vec.begin(), vec.end(), task_location);
    }
    template <class TIter>
        requires std::is_trivially_copyable_v<std::remove_reference_t<decltype(*TIter())>>
    void set(
        const TIter &begin,
        const TIter &end,
        TaskLocation task_location)
    {
        set_type_erased(
            reinterpret_cast<const char *>(&*begin),
            reinterpret_cast<const char *>(&*end),
            task_location);
    }
    template <typename T, size_t size>
    void set(const T (&array)[size], TaskLocation task_location) {
        set(array + 0, array + size, task_location);
    }
    virtual ~IArrayBuffer() = default;
    virtual bool copy_in_progress() const = 0;
    virtual void wait() const = 0;
    virtual void update() = 0;
    virtual void bind() const = 0;
    virtual bool is_awaited() const = 0;
    virtual std::shared_ptr<IArrayBuffer> fork() = 0;
protected:
    virtual void set_type_erased(
        const char* begin,
        const char* end,
        TaskLocation task_location) = 0;
};

}
