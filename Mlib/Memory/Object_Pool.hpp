#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Source_Location.hpp>
#include <compare>
#include <cstdlib>
#include <mutex>
#include <set>

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
    void* buffer;
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
    T& create(SourceLocation loc, Args&&... args) {
        auto* buf = std::malloc(sizeof(T));
        T* o;
        try {
            o = new (buf) T(std::forward<Args>(args)...);
        } catch (...) {
            std::free(buf);
            throw;
        }
        add(buf, *o, loc);
        return *o;
    }
    void remove(Object* o);
    void remove(Object& o);
    void clear();
    void assert_no_leaks() const;
private:
    void add(void* buffer, Object& o, SourceLocation loc);
    void delete_(const ObjectAndSourceLocation& o);
    mutable std::mutex mutex_;
    InObjectPoolDestructor what_to_do_in_dtor_;
    std::set<ObjectAndSourceLocation> ptrs_;
    std::set<Object*> deleting_ptrs_;
    bool clearing_;
};

MLIB_MEMORY_API extern ObjectPool global_object_pool;

};
