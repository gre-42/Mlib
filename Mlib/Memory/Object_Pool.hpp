#pragma once
#include <Mlib/Object.hpp>
#include <Mlib/Source_Location.hpp>
#include <compare>
#include <mutex>
#include <set>

namespace Mlib {

struct ObjectAndSourceLocation {
    Object* object;
    SourceLocation loc;
    inline std::strong_ordering operator <=> (const ObjectAndSourceLocation& other) const {
        return object <=> other.object;
    }
};

class ObjectPool {
    ObjectPool(const ObjectPool&) = delete;
    ObjectPool& operator = (const ObjectPool&) = delete;
public:
    ObjectPool();
    ~ObjectPool();
    template<class T, class... Args>
    T& create(SourceLocation loc, Args&&... args) {
        auto* buf = new char[sizeof(T)];
        T* o;
        try {
            o = new (buf) T(std::forward<Args>(args)...);
        } catch (...) {
            delete[] buf;
            throw;
        }
        add(*o, loc);
        return *o;
    }
    void add(Object* o, SourceLocation loc);
    void add(Object& o, SourceLocation loc);
    void remove(Object* o);
    void remove(Object& o);
    void clear();
private:
    void delete_(const ObjectAndSourceLocation& o);
    mutable std::mutex mutex_;
    std::set<ObjectAndSourceLocation> ptrs_;
    std::set<Object*> deleting_ptrs_;
    bool clearing_;
};

};
