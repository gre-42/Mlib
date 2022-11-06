#pragma once

namespace Mlib {

template<class TContainer>
class Iterable {
public:
    using iterator = TContainer::iterator;
    using reverse_iterator = TContainer::reverse_iterator;

    explicit Iterable(TContainer& container)
    : container_{container}
    {}

    iterator begin() const
    {
        return container_.begin();
    }

    iterator end() const
    {
        return container_.end();
    }

    reverse_iterator rbegin() const
    {
        return container_.rbegin();
    }

    reverse_iterator rend() const
    {
        return container_.rend();
    }
private:
    TContainer& container_;
};

}
