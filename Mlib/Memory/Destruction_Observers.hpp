#pragma once
#include <atomic>
#include <mutex>
#include <set>

namespace Mlib {

template <class T>
class DestructionObserver;

enum class ObserverAlreadyExistsBehavior {
    IGNORE,
    RAISE
};

enum class ObserverDoesNotExistBehavior {
    IGNORE,
    RAISE
};

template <class T>
class DestructionObservers {
public:
    explicit DestructionObservers(T obj);
    ~DestructionObservers();

    void add(DestructionObserver<T>& destruction_observer,
             ObserverAlreadyExistsBehavior already_exists_behavior = ObserverAlreadyExistsBehavior::RAISE);
    void remove(DestructionObserver<T>& destruction_observer,
                ObserverDoesNotExistBehavior does_not_exist_behavior = ObserverDoesNotExistBehavior::RAISE);
    bool shutting_down() const;
    void shutdown();
    void notify_destroyed();
private:
    void shutdown_unsafe();
    std::set<DestructionObserver<T>*> observers_;
    mutable std::mutex mutex_;
    std::atomic_bool shutting_down_;
    T obj_;
};

}
