#pragma once
#include <mutex>
#include <set>

namespace Mlib {

class DestructionObserver;
class Object;

class DestructionObservers {
public:
    explicit DestructionObservers(Object* obj);
    ~DestructionObservers();

    void add(DestructionObserver* destruction_observer,
             bool ignore_exists = false);
    void remove(DestructionObserver* destruction_observer,
                bool ignore_not_exists = false);
    bool shutting_down() const;
    void shutdown();
private:
    std::set<DestructionObserver*> observers_;
    mutable std::mutex mutex_;
    bool shutting_down_;
    Object* obj_;
};

}
