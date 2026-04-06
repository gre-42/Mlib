#pragma once
#include <list>
#include <memory>
#include <utility>

namespace Mlib {

template <class First, class Second>
class ReadonlyListOfSharedPairs {
    template <class TMutex, class TBaseList>
    friend class GenericThreadSafeListOfSharedPairs;
public:
    using Pair = std::pair<First, Second>;
    using Container = std::list<std::shared_ptr<Pair>>;
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
