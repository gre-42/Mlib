#pragma once
#include <Mlib/List_Of_Pairs/Readonly_List_Of_Shared_Pairs.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Type_Traits/Remove_Const.hpp>
#include <mutex>

namespace Mlib {

template <class TMutex, class TBaseList>
class GenericThreadSafeListOfSharedPairs {
public:
    using Pair = TBaseList::Pair;
    using value_type = TBaseList::value_type;

    GenericThreadSafeListOfSharedPairs() = default;
    ~GenericThreadSafeListOfSharedPairs() = default;
    GenericThreadSafeListOfSharedPairs(GenericThreadSafeListOfSharedPairs&& other) {
        // this->mutex_ need not be locked in the ctor.
        std::scoped_lock lock{ other.mutex_ };
        remove_const(container_.container_) = std::move(remove_const(other.container_.container_));
    }
    GenericThreadSafeListOfSharedPairs& operator = (GenericThreadSafeListOfSharedPairs&& other) {
        std::scoped_lock lock{ mutex_, other.mutex_ };
        remove_const(container_.container_) = std::move(remove_const(other.container_.container_));
        return *this;
    }
    template <class... TArgs>
    decltype(auto) emplace_back(TArgs&&... args) {
        std::scoped_lock lock{ mutex_ };
        return remove_const(container_.container_).emplace_back(std::make_shared<Pair>(std::forward<TArgs>(args)...));
    }
    template <class TArgs>
    void push_back(TArgs&& arg) {
        std::scoped_lock lock{ mutex_ };
        remove_const(container_.container_).push_back(std::forward<TArgs>(arg));
    }
    void clear() {
        std::scoped_lock lock{ mutex_ };
        remove_const(container_.container_).clear();
    }
    TBaseList iterable() {
        std::shared_lock lock{ mutex_ };
        return container_;
    }
    const TBaseList iterable() const {
        return const_cast<GenericThreadSafeListOfSharedPairs*>(this)->iterable();
    }
    bool empty() const {
        return container_.container_.empty();
    }
    std::size_t size() const {
        return container_.container_.size();
    }
private:
    mutable TMutex mutex_;
    TBaseList container_;
};

template <class First, class Second>
using ThreadSafeListOfSharedPairs = GenericThreadSafeListOfSharedPairs<SafeAtomicRecursiveSharedMutex, ReadonlyListOfSharedPairs<First, Second>>;

}
