#pragma once
#include <list>
#include <utility>

namespace Mlib {

template <class First, class Second>
class PairAdapter {
public:
    template <class First2, class Second2>
    PairAdapter(First2&& first, Second2&& second)
        : pair_{std::forward<First2>(first), std::forward<Second2>(second)}
    {}
    std::pair<First, Second>* operator -> () {
        return &pair_;
    }
    const std::pair<First, Second>* operator -> () const {
        return &pair_;
    }
private:
    std::pair<First, Second> pair_;
};

template <class First, class Second>
class ReadonlyListOfPairAdapters {
    template <class TBaseList>
    friend class GenericListOfPairAdapters;
public:
    using Container = std::list<PairAdapter<First, Second>>;
    using value_type = Container::value_type;

    decltype(auto) begin() {
        return const_cast<Container&>(container_).begin();
    }
    decltype(auto) end() {
        return const_cast<Container&>(container_).end();
    }
    decltype(auto) begin() const {
        return container_.begin();
    }
    decltype(auto) end() const {
        return container_.end();
    }
private:
    // const to delete the move operator
    const Container container_;
};

}
