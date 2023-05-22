#pragma once

namespace Mlib {

template<class TContainer>
class IterableWrapper {
public:
    using iterator = typename TContainer::iterator;
    using reverse_iterator = typename TContainer::reverse_iterator;

    explicit IterableWrapper(TContainer& container)
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
