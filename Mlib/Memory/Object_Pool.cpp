#include "Object_Pool.hpp"
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ObjectPool::ObjectPool()
    : clearing_{ false }
{}

ObjectPool::~ObjectPool() {
    clear();
}

void ObjectPool::add(Object& o, SourceLocation loc) {
    std::scoped_lock lock{ mutex_ };
    if (clearing_) {
        verbose_abort("ObjectPool::add called during clearing");
    }
    if (!ptrs_.emplace(&o, loc).second) {
        THROW_OR_ABORT("Unique pointer alerady exists");
    }
}

void ObjectPool::add(Object* o, SourceLocation loc) {
    if (o == nullptr) {
        verbose_abort("Attempt to add nullptr");
    }
    add(*o, loc);
}

void ObjectPool::remove(Object& o) {
    std::scoped_lock lock{ mutex_ };
    if (deleting_ptrs_.contains(&o)) {
        return;
    }
    auto n = ptrs_.extract({ &o, CURRENT_SOURCE_LOCATION });
    if (n.empty()) {
        verbose_abort("Could not erase unique ptr");
    }
    delete_(n.value());
}

void ObjectPool::remove(Object* o) {
    if (o == nullptr) {
        verbose_abort("Attempt to remove nullptr");
    }
    remove(*o);
}

void ObjectPool::delete_(const ObjectAndSourceLocation& o) {
    // linfo() << "Deleting " << o.loc.file_name() << ':' << o.loc.line();
    deleting_ptrs_.insert(o.object);
    std::exception_ptr eptr = nullptr;
    try {
        o.object->~Object();
    } catch (...) {
        lwarn() << "Destructor threw an exception";
        eptr = std::current_exception(); 
    }
    if (deleting_ptrs_.erase(o.object) != 1) {
        verbose_abort("Could not erase from deleting_ptrs");
    }
    delete[] reinterpret_cast<char*>(o.object);
    if (eptr != nullptr) {
        std::rethrow_exception(eptr);
    }
}

void ObjectPool::clear() {
    std::scoped_lock lock{ mutex_ };
    if (clearing_) {
        verbose_abort("ObjectPool already clearing");
    }
    clearing_ = true;
    clear_set_recursively(ptrs_, [this](auto& o) { delete_(o); });
    clearing_ = false;
}
