#include "Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stdexcept>

using namespace Mlib;

DestructionObservers::DestructionObservers(Object& obj)
: shutting_down_{false},
  obj_{&obj}
{}

DestructionObservers::~DestructionObservers() {
    if (!shutting_down_) {
        shutdown();
    }
}

bool DestructionObservers::shutting_down() const {
    return shutting_down_;
}

void DestructionObservers::add(
    DestructionObserver& destruction_observer,
    ObserverAlreadyExistsBehavior already_exists_behavior)
{
    std::scoped_lock lock{mutex_};
    auto r = observers_.insert(&destruction_observer);
    if (!r.second && (already_exists_behavior == ObserverAlreadyExistsBehavior::RAISE)) {
        THROW_OR_ABORT("Destruction observer already registered");
    }
}

void DestructionObservers::remove(
    DestructionObserver& destruction_observer,
    ObserverDoesNotExistBehavior does_not_exist_behavior)
{
    std::scoped_lock lock{mutex_};
    if (!shutting_down()) {
        size_t nerased = observers_.erase(&destruction_observer);
        if ((nerased != 1) && (does_not_exist_behavior == ObserverDoesNotExistBehavior::RAISE)) {
            THROW_OR_ABORT("Could not find destruction observer to be erased");
        }
    }
}

void DestructionObservers::shutdown() {
    if (shutting_down_) {
        THROW_OR_ABORT("Already shutting down");
    }
    shutting_down_ = true;
    clear_set_recursively(observers_, [this](DestructionObserver* obs){
        obs->notify_destroyed(*obj_);
    });
}
