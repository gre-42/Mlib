#include "Object_Pool.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <exception>

using namespace Mlib;

ObjectPool Mlib::global_object_pool{ InObjectPoolDestructor::ASSERT_NO_LEAKS };

ObjectPool::ObjectPool(InObjectPoolDestructor what_to_do_in_dtor)
    : what_to_do_in_dtor_{ what_to_do_in_dtor }
    , clearing_{ false }
{}

ObjectPool::~ObjectPool() {
    if (!deleting_ptrs_.empty()) {
        verbose_abort("ObjectPool dtor called during deletion");
    }
    switch (what_to_do_in_dtor_) {
    case InObjectPoolDestructor::CLEAR:
        clear();
        break;
    case InObjectPoolDestructor::ASSERT_NO_LEAKS:
        assert_no_leaks();
        break;
    default:
        verbose_abort("Unknown InObjectPoolDestructor value");
    }
}

void ObjectPool::add(std::function<void()> deallocate, Object& o, SourceLocation loc) {
    std::scoped_lock lock{ mutex_ };
    if (clearing_) {
        verbose_abort("ObjectPool::add called during clearing");
    }
    if (!ptrs_.emplace(std::move(deallocate), &o, loc).second) {
        THROW_OR_ABORT("Unique pointer already exists");
    }
}

// void ObjectPool::add(Object* o, SourceLocation loc) {
//     if (o == nullptr) {
//         verbose_abort("Attempt to add nullptr");
//     }
//     add(*o, loc);
// }

void ObjectPool::remove(Object& o) {
    std::unique_lock lock{ mutex_ };
    if (deleting_ptrs_.contains(&o)) {
        return;
    }
    auto n = ptrs_.extract({ nullptr, &o, CURRENT_SOURCE_LOCATION });
    if (n.empty()) {
        verbose_abort("ObjectPool: Could not remove object");
    }
    lock.unlock();
    delete_(n.value());
}

void ObjectPool::remove(Object* o) {
    if (o == nullptr) {
        verbose_abort("ObjectPool: Attempt to remove nullptr");
    }
    remove(*o);
}

void ObjectPool::delete_(const ObjectAndSourceLocation& o) {
    {
        std::scoped_lock lock{ mutex_ };
        if (!deleting_ptrs_.insert(o.object).second) {
            verbose_abort("Could not insert into deleting_ptrs");
        }
    }
    std::exception_ptr eptr = nullptr;
    try {
        o.object->~Object();
    } catch (...) {
        lwarn() << "Destructor threw an exception";
        eptr = std::current_exception(); 
    }
    {
        std::scoped_lock lock{ mutex_ };
        if (deleting_ptrs_.erase(o.object) != 1) {
            verbose_abort("Could not erase from deleting_ptrs");
        }
    }
    o.deallocate();
    if (eptr != nullptr) {
        std::rethrow_exception(eptr);
    }
}

void ObjectPool::clear() {
    std::unique_lock lock{ mutex_ };
    if (clearing_) {
        verbose_abort("ObjectPool already clearing");
    }
    clearing_ = true;
    clear_set_recursively_with_lock(ptrs_, lock, [this](auto& o) { delete_(o); });
    clearing_ = false;
}

void ObjectPool::assert_no_leaks() const {
    if (!ptrs_.empty()) {
        for (const auto& p : ptrs_) {
            lerr() << p.loc.file_name() << ':' << p.loc.line();
        }
        verbose_abort("Memory leaks detected in ObjctPool");
    }
}
