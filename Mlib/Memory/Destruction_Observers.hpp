#pragma once
#include <Mlib/Threads/Atomic_Mutex.hpp>
#include <atomic>
#include <set>

namespace Mlib {

template <class T>
class DestructionObserver;
template <class T>
class DanglingBaseClassPtr;
template <class T>
class DanglingBaseClassRef;

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

    void add(const DanglingBaseClassRef<DestructionObserver<T>>& destruction_observer,
             ObserverAlreadyExistsBehavior already_exists_behavior = ObserverAlreadyExistsBehavior::RAISE);
    void remove(const DanglingBaseClassRef<DestructionObserver<T>>& destruction_observer,
                ObserverDoesNotExistBehavior does_not_exist_behavior = ObserverDoesNotExistBehavior::RAISE);
    bool clearing() const;
    void clear();
private:
    std::set<DanglingBaseClassPtr<DestructionObserver<T>>> observers_;
    mutable AtomicMutex mutex_;
    std::atomic_bool clearing_;
    T obj_;
};

}
