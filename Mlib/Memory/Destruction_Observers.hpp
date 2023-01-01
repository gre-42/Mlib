#pragma once
#include <mutex>
#include <set>

namespace Mlib {

class DestructionObserver;
class Object;

enum class ObserverAlreadyExistsBehavior {
    IGNORE,
    RAISE
};

enum class ObserverDoesNotExistBehavior {
    IGNORE,
    RAISE
};

class DestructionObservers {
public:
    explicit DestructionObservers(Object* obj);
    ~DestructionObservers();

    void add(DestructionObserver* destruction_observer,
             ObserverAlreadyExistsBehavior already_exists_behavior = ObserverAlreadyExistsBehavior::RAISE);
    void remove(DestructionObserver* destruction_observer,
                ObserverDoesNotExistBehavior does_not_exist_behavior = ObserverDoesNotExistBehavior::RAISE);
    bool shutting_down() const;
    void shutdown();
private:
    std::set<DestructionObserver*> observers_;
    mutable std::mutex mutex_;
    bool shutting_down_;
    Object* obj_;
};

}
