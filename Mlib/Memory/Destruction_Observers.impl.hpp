#include "Destruction_Observers.hpp"
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Threads/Unlock_Guard.hpp>

namespace Mlib {

template <class T>
DestructionObservers<T>::DestructionObservers(T obj)
    : clearing_{ false }
    , obj_{ obj }
{}

template <class T>
DestructionObservers<T>::~DestructionObservers() {
    if (!observers_.empty()) {
        verbose_abort("DestructionObservers not empty in its destructor, please call \"clear()\" in your destructor");
    }
}

template <class T>
bool DestructionObservers<T>::clearing() const {
    return clearing_;
}

template <class T>
void DestructionObservers<T>::add(
    const DanglingBaseClassRef<DestructionObserver<T>>& destruction_observer,
    ObserverAlreadyExistsBehavior already_exists_behavior)
{
    if (clearing_) {
        verbose_abort("DestructionObservers::add despite clearing");
    }
    std::scoped_lock lock{ mutex_ };
    auto r = observers_.insert(destruction_observer.ptr());
    if (!r.second && (already_exists_behavior == ObserverAlreadyExistsBehavior::RAISE)) {
        verbose_abort("Destruction observer already registered");
    }
}

template <class T>
void DestructionObservers<T>::remove(
    const DanglingBaseClassRef<DestructionObserver<T>>& destruction_observer,
    ObserverDoesNotExistBehavior does_not_exist_behavior)
{
    std::unique_lock lock{ mutex_, std::defer_lock };
    if (!clearing_) {
        lock.lock();
    }
    size_t nerased = observers_.erase(destruction_observer.ptr());
    if ((nerased != 1) && (does_not_exist_behavior == ObserverDoesNotExistBehavior::RAISE)) {
        verbose_abort("Could not find destruction observer to be erased");
    }
}

template <class T>
void DestructionObservers<T>::clear() {
    if (clearing_) {
        verbose_abort("DestructionObservers::clear despite active clearing");
    }
    std::unique_lock lock{ mutex_ };
    clearing_ = true;
    clear_set_recursively(observers_, [this, &lock](DanglingBaseClassPtr<DestructionObserver<T>>& obs){
        UnlockGuard ulock{ lock };
        auto obs0 = obs.get();
        obs = nullptr;
        obs0->notify_destroyed(obj_);
    });
    clearing_ = false;
}

}
