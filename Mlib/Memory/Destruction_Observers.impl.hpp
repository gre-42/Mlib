#include "Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Memory/Recursive_Deletion.hpp>
#include <Mlib/Os/Os.hpp>

namespace Mlib {

template <class T>
DestructionObservers<T>::DestructionObservers(T obj)
    : shutting_down_{ false }
    , obj_{ obj }
{}

template <class T>
DestructionObservers<T>::~DestructionObservers() {
    if (!shutting_down_) {
        shutdown_unsafe();
    }
}

template <class T>
bool DestructionObservers<T>::shutting_down() const {
    return shutting_down_;
}

template <class T>
void DestructionObservers<T>::add(
    DestructionObserver<T>& destruction_observer,
    ObserverAlreadyExistsBehavior already_exists_behavior)
{
    if (shutting_down_) {
        verbose_abort("DestructionObservers::add despite shutdown");
    }
    std::scoped_lock lock{ mutex_ };
    auto r = observers_.insert(&destruction_observer);
    if (!r.second && (already_exists_behavior == ObserverAlreadyExistsBehavior::RAISE)) {
        verbose_abort("Destruction observer already registered");
    }
}

template <class T>
void DestructionObservers<T>::remove(
    DestructionObserver<T>& destruction_observer,
    ObserverDoesNotExistBehavior does_not_exist_behavior)
{
    std::unique_lock lock{ mutex_, std::defer_lock };
    if (!shutting_down_) {
        lock.lock();
    }
    size_t nerased = observers_.erase(&destruction_observer);
    if ((nerased != 1) && (does_not_exist_behavior == ObserverDoesNotExistBehavior::RAISE)) {
        verbose_abort("Could not find destruction observer to be erased");
    }
}

template <class T>
void DestructionObservers<T>::shutdown() {
    if (shutting_down_) {
        verbose_abort("DestructionObservers::shutdown despite active shutdown");
    }
    std::unique_lock lock{ mutex_ };
    shutting_down_ = true;
    clear_set_recursively(observers_, [this, &lock](DestructionObserver<T>* obs){
        lock.unlock();
        obs->notify_destroyed(obj_);
        lock.lock();
    });
}

template <class T>
void DestructionObservers<T>::notify_destroyed() {
    if (shutting_down_) {
        verbose_abort("DestructionObservers::notify_destroyed despite shutdown");
    }
    std::unique_lock lock{ mutex_ };
    shutting_down_ = true;
    clear_set_recursively(observers_, [this, &lock](DestructionObserver<T>* obs){
        lock.unlock();
        obs->notify_destroyed(obj_);
        lock.lock();
    });
    shutting_down_ = false;
}

template <class T>
void DestructionObservers<T>::shutdown_unsafe() {
    if (shutting_down_) {
        verbose_abort("DestructionObservers::shutdown_unsafe despite shutdown");
    }
    shutting_down_ = true;
    clear_set_recursively(observers_, [this](DestructionObserver<T>* obs){
        obs->notify_destroyed(obj_);
    });
}

}
