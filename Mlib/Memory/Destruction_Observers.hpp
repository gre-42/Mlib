#pragma once
#include <atomic>
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
    explicit DestructionObservers(const Object& obj);
    ~DestructionObservers();

    void add(DestructionObserver& destruction_observer,
             ObserverAlreadyExistsBehavior already_exists_behavior = ObserverAlreadyExistsBehavior::RAISE);
    void remove(DestructionObserver& destruction_observer,
                ObserverDoesNotExistBehavior does_not_exist_behavior = ObserverDoesNotExistBehavior::RAISE);
    bool shutting_down() const;
    void shutdown();
    void notify_destroyed();
private:
    void send_shutdown_messages();
    std::set<DestructionObserver*> observers_;
    mutable std::mutex mutex_;
    std::atomic_bool shutting_down_;
    const Object& obj_;
};

}
