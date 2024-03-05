#pragma once

namespace Mlib {

class IArrayBuffer {
public:
    template <class TData>
    void reserve(size_t size) {
        set((TData *)nullptr, (TData *)nullptr + size);
    }
    template <class TData>
    void set(const std::vector<TData> &vec) {
        set(vec.begin(), vec.end());
    }
    template <class TIter>
        requires std::is_trivially_copyable_v<std::remove_reference_t<decltype(*TIter())>>
    void set(const TIter &begin, const TIter &end) {
        set_type_erased(reinterpret_cast<const char *>(&*begin),
            reinterpret_cast<const char *>(&*end));
    }
    template <typename T, size_t size>
    void set(const T (&array)[size]) {
        set(array + 0, array + size);
    }
    virtual ~IArrayBuffer() = default;
    virtual bool copy_in_progress() const = 0;
	virtual void wait() const = 0;
    virtual void update() = 0;
    virtual void bind() const = 0;
protected:
    virtual void set_type_erased(const char* begin, const char* end) = 0;
};

}
