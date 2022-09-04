#include "Destruction_Observers.hpp"
#include <Mlib/Memory/Destruction_Observer.hpp>
#include <Mlib/Recursive_Deletion.hpp>
#include <stdexcept>

using namespace Mlib;

DestructionObservers::DestructionObservers(Object* obj)
: shutting_down_{false},
  obj_{obj}
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
    DestructionObserver* destruction_observer,
    bool ignore_exists)
{
    std::unique_lock lock{mutex_};
    auto r = observers_.insert(destruction_observer);
    if (!ignore_exists && !r.second) {
        throw std::runtime_error("Destruction observer already registered");
    }
}

void DestructionObservers::remove(
    DestructionObserver* destruction_observer,
    bool ignore_not_exists)
{
    std::unique_lock lock{mutex_};
    if (!shutting_down()) {
        size_t nerased = observers_.erase(destruction_observer);
        if (!ignore_not_exists && (nerased != 1)) {
            throw std::runtime_error("Could not find destruction observer to be erased");
        }
    }
}

void DestructionObservers::shutdown() {
    if (shutting_down_) {
        throw std::runtime_error("Already shutting down");
    }
    shutting_down_ = true;
    clear_set_recursively(observers_, [this](const auto& obs){
        obs->notify_destroyed(obj_);
    });
}
