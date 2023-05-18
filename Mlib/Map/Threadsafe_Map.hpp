#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

template <class T>
class ThreadsafeMap {
public:
    explicit ThreadsafeMap(RecursiveSharedMutex& mutex)
    : mutex_{mutex}
    {}

    void insert(const std::string& name, const T& value) {
        std::scoped_lock lock{mutex_};
        if (!elements_.insert({name, value}).second) {
            THROW_OR_ABORT("Element with name \"" + name + "\" already exists");
        }
    }

    T& get(const std::string& name) {
        std::shared_lock lock{mutex_};
        auto it = elements_.find(name);
        if (it == elements_.end()) {
            THROW_OR_ABORT("Element with name \"" + name + "\" does not exist");
        }
        return it->second;
    }

    const T& get(const std::string& name) const {
        return const_cast<ThreadsafeMap*>(this)->get(name);
    }

    auto begin() { return elements_.begin(); }
    auto end() { return elements_.end(); }
    auto begin() const { return elements_.begin(); }
    auto end() const { return elements_.end(); }
private:
    RecursiveSharedMutex& mutex_;
    std::map<std::string, T> elements_;
};

}
