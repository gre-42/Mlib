#pragma once
#include <Mlib/Threads/Safe_Recursive_Shared_Mutex.hpp>
#include <functional>
#include <map>
#include <mutex>
#include <string>

namespace Mlib {

template <class T>
class ThreadsafeCreatorMap {
public:
    explicit ThreadsafeCreatorMap(SafeAtomicRecursiveSharedMutex& mutex)
        : mutex_{ mutex }
    {}

    void insert_creator(
        const std::string& name,
        const std::function<T()>& creator)
    {
        std::scoped_lock lock{mutex_};
        if (elements_.contains(name)) {
            THROW_OR_ABORT("Element with name \"" + name + "\" already exists");
        }
        if (!creators_.insert({name, creator}).second) {
            THROW_OR_ABORT("Element creator with name \"" + name + "\" already exists");
        }
    }

    T& get(const std::string& name)
    {
        {
            std::shared_lock lock{mutex_};
            auto it = elements_.find(name);
            if (it != elements_.end()) {
                return *it->second;
            }
        }
        {
            std::scoped_lock lock{mutex_};
            auto it = creators_.find(name);
            if (it != creators_.end()) {
                auto iit = elements_.insert({name, std::move(it->second())});
                if (!iit.second) {
                    THROW_OR_ABORT("Recursive dependency: Element and with name \"" + name + "\" exists");
                }
                creators_.erase(it);
                return *iit.first->second;
            }
        }
        THROW_OR_ABORT("Could not find element or creator with name \"" + name + '"');
    }

    decltype(auto) begin() { return elements_.begin(); }
    decltype(auto) end() { return elements_.end(); }
    decltype(auto) begin() const { return elements_.begin(); }
    decltype(auto) end() const { return elements_.end(); }
private:
    SafeAtomicRecursiveSharedMutex& mutex_;
    std::map<std::string, T> elements_;
    std::map<std::string, std::function<T()>> creators_;
};

}
