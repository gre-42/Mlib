#pragma once
#include <cstddef>
#include <memory>
#include <vector>

namespace Mlib {

class IArrayBuffer {
public:
    // init
    template <class TData>
    void reserve(size_t size) {
        init((TData*)nullptr, (TData*)nullptr + size);
    }
    template <class TData>
    void init(const std::vector<TData>& vec) {
        init(vec.begin(), vec.end());
    }
    template <class TIter>
        requires std::is_trivially_copyable_v<std::remove_reference_t<decltype(*TIter())>>
    void init(
        const TIter& begin,
        const TIter& end)
    {
        init_type_erased(
            reinterpret_cast<const std::byte *>(&*begin),
            reinterpret_cast<const std::byte *>(&*end));
    }
    template <typename T, size_t size>
    void init(const T (&array)[size]) {
        init(array + 0, array + size);
    }
    // substitute
    template <class TData>
    void substitute(const std::vector<TData>& vec) {
        substitute(vec.begin(), vec.end());
    }
    template <class TIter>
        requires std::is_trivially_copyable_v<std::remove_reference_t<decltype(*TIter())>>
    void substitute(
        const TIter& begin,
        const TIter& end)
    {
        substitute_type_erased(
            reinterpret_cast<const std::byte *>(&*begin),
            reinterpret_cast<const std::byte *>(&*end));
    }
    template <typename T, size_t size>
    void substitute(const T (&array)[size]) {
        substitute(array + 0, array + size);
    }
    // misc
    virtual ~IArrayBuffer() = default;
    virtual bool copy_in_progress() const = 0;
    virtual void wait() const = 0;
    virtual void update() = 0;
    virtual void bind() const = 0;
    virtual bool is_initialized() const = 0;
    virtual std::shared_ptr<IArrayBuffer> fork() = 0;
protected:
    virtual void init_type_erased(
        const std::byte* begin,
        const std::byte* end) = 0;
    virtual void substitute_type_erased(
        const std::byte* begin,
        const std::byte* end) = 0;
};

}
