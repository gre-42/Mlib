#pragma once
#include <Mlib/List_Of_Pairs/Readonly_List_Of_Pair_Adapters.hpp>
#include <Mlib/Type_Traits/Remove_Const.hpp>

namespace Mlib {

template <class TBaseList>
class GenericListOfPairAdapters {
public:
    using value_type = TBaseList::value_type;

    GenericListOfPairAdapters() = default;
    ~GenericListOfPairAdapters() = default;
    GenericListOfPairAdapters(GenericListOfPairAdapters&& other) {
        remove_const(container_.container_) = std::move(remove_const(other.container_.container_));
    }
    GenericListOfPairAdapters& operator = (GenericListOfPairAdapters&& other) {
        remove_const(container_.container_) = std::move(remove_const(other.container_.container_));
        return *this;
    }
    template <class... TArgs>
    decltype(auto) emplace_back(TArgs&&... args) {
        return remove_const(container_.container_).emplace_back(std::forward<TArgs>(args)...);
    }
    template <class TArgs>
    void push_back(TArgs&& arg) {
        remove_const(container_.container_).push_back(std::forward<TArgs>(arg));
    }
    void clear() {
        remove_const(container_.container_).clear();
    }
    TBaseList& iterable() {
        return container_;
    }
    const TBaseList& iterable() const {
        return const_cast<GenericListOfPairAdapters*>(this)->iterable();
    }
    bool empty() const {
        return container_.container_.empty();
    }
    std::size_t size() const {
        return container_.container_.size();
    }
private:
    TBaseList container_;
};

template <class First, class Second>
using ListOfPairAdapters = GenericListOfPairAdapters<ReadonlyListOfPairAdapters<First, Second>>;

}
