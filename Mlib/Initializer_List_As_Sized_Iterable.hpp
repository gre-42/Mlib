#pragma once

namespace Mlib {

template <class TData>
class InitializerListAsSizedIterable {
public:
    typedef typename std::initializer_list<TData>::value_type value_type;
    typedef typename std::initializer_list<TData>::iterator iterator;
    typedef typename std::initializer_list<TData>::const_iterator const_iterator;
    typedef typename std::initializer_list<TData>::size_type size_type;
    InitializerListAsSizedIterable(std::initializer_list<TData> data)
    : data_(data)
    {}
    iterator begin() {
        return data_.begin();
    }
    const_iterator begin() const {
        return data_.begin();
    }
    iterator end() {
        return data_.end();
    }
    const_iterator end() const {
        return data_.end();
    }
    size_type size() const {
        return data_.size();
    }
    bool empty() const {
        return data_.empty();
    }
private:
    std::initializer_list<TData> data_;
};

}
