#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Iterator/Guarded_Iterable.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <functional>
#include <mutex>
#include <stdexcept>
#include <string>
#include <unordered_map>

namespace Mlib {

template <class T>
class ThreadsafeDefaultMap {
    using TBaseMap = std::unordered_map<VariableAndHash<std::string>, T>;
    using TMutex = SafeAtomicRecursiveSharedMutex;
public:
    explicit ThreadsafeDefaultMap(
        std::function<T(const VariableAndHash<std::string>&)> deflt)
        : default_{ std::move(deflt) }
    {}

    void insert(const VariableAndHash<std::string>& name, const T& value) {
        std::scoped_lock lock{mutex_};
        if (!elements_.insert({name, value}).second) {
            throw std::runtime_error("Element with name \"" + *name + "\" already exists");
        }
    }

    T& get(const VariableAndHash<std::string>& name) {
        {
            std::shared_lock lock{mutex_};
            auto it = elements_.find(name);
            if (it != elements_.end()) {
                return it->second;
            }
        }
        std::scoped_lock lock{mutex_};
        auto it = elements_.find(name);
        if (it != elements_.end()) {
            return it->second;
        }
        auto iit = elements_.insert({name, default_(name)});
        if (!iit.second) {
            throw std::runtime_error("Recursive insertion: Element with name \"" + *name + "\" already exists");
        }
        return iit.first->second;
    }

    const T& get(const VariableAndHash<std::string>& name) const {
        return const_cast<ThreadsafeDefaultMap*>(this)->get(name);
    }

    GuardedIterable<typename TBaseMap::iterator, std::scoped_lock<TMutex>> scoped() {
        return { mutex_, elements_.begin(), elements_.end() };
    }
    GuardedIterable<typename TBaseMap::iterator, std::shared_lock<TMutex>> shared() {
        return { mutex_, elements_.begin(), elements_.end() };
    }
    GuardedIterable<typename TBaseMap::const_iterator, std::scoped_lock<TMutex>> scoped() const {
        return { mutex_, elements_.begin(), elements_.end() };
    }
    GuardedIterable<typename TBaseMap::const_iterator, std::shared_lock<TMutex>> shared() const {
        return { mutex_, elements_.begin(), elements_.end() };
    }
private:
    mutable TMutex mutex_;
    TBaseMap elements_;
    std::function<T(const VariableAndHash<std::string>&)> default_;
};

}
