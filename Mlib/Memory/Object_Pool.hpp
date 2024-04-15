#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Source_Location.hpp>
#include <compare>
#include <functional>
#include <mutex>
#include <set>
#include <type_traits>
#include <unordered_set>

#ifdef _MSC_VER
#ifdef MlibMemory_EXPORTS
#define MLIB_MEMORY_API __declspec(dllexport)
#else
#define MLIB_MEMORY_API __declspec(dllimport)
#endif
#else
#define MLIB_MEMORY_API
#endif

namespace Mlib {

struct ObjectAndSourceLocation {
    std::function<void()> deallocate;
    Object* object;
    SourceLocation loc;
    inline std::strong_ordering operator <=> (const ObjectAndSourceLocation& other) const {
        return object <=> other.object;
    }
};

enum class InObjectPoolDestructor {
    CLEAR,
    ASSERT_NO_LEAKS
};

class ObjectPool {
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator = (const ObjectPool&) = delete;
public:
    ObjectPool(InObjectPoolDestructor what_to_do_in_dtor);
    ~ObjectPool();
    template<class T, class... Args>
        requires std::is_convertible_v<T&, Object&>
    T& create(SourceLocation loc, Args&&... args) {
        T* o = std::allocator<T>().allocate(1);
        try {
            new (o) T(std::forward<Args>(args)...);
        } catch (...) {
            std::allocator<T>().deallocate(o, 1);
            throw;
        }
        add([o](){ std::allocator<T>().deallocate(o, 1); }, *o, loc);
        return *o;
    }
    template<class T>
        requires std::is_convertible_v<T&, Object&>
    T& add(std::unique_ptr<T>&& u, SourceLocation loc) {
        if (u == nullptr) {
            verbose_abort("Attempt to add nullptr to object pool");
        }
        auto o = u.release();
        add([o]() { std::allocator<T>().deallocate(o, 1); }, *o, loc);
        return *o;
    }
    void remove(Object* o);
    void remove(Object& o);
    void clear();
    void assert_no_leaks() const;
private:
    void add(std::function<void()> deallocate, Object& o, SourceLocation loc);
    void delete_(const ObjectAndSourceLocation& o);
    mutable std::mutex mutex_;
    InObjectPoolDestructor what_to_do_in_dtor_;
    std::set<ObjectAndSourceLocation> ptrs_;
    std::unordered_set<Object*> deleting_ptrs_;
    bool clearing_;
};

MLIB_MEMORY_API extern ObjectPool global_object_pool;

};
