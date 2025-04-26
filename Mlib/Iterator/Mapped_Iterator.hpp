#pragma once

namespace Mlib {

template <class TIterator, class TMapping>
class MappedIterator {
public:
    explicit MappedIterator(TIterator it, TMapping mapping)
        : it_{ std::move(it) }
        , mapping_{ std::move(mapping) }
    {}
    decltype(auto) operator * () const {
        return *mapping_(*it_);
    }
    const auto* operator -> () const {
        return mapping_(*it_);
    }
    MappedIterator& operator ++ () {
        ++it_;
        return *this;
    }
    bool operator == (const MappedIterator& other) const {
        return it_ == other.it_;
    }
    bool operator != (const MappedIterator& other) const {
        return it_ != other.it_;
    }
private:
    TIterator it_;
    TMapping mapping_;
};

}
