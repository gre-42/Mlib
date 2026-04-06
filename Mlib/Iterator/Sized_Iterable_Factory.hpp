#pragma once
#include <cstddef>

namespace Mlib {

template<class TBegin, class TEnd, class TSize>
class SizedIterableFactory {
public:
    using iterator = TBegin;
    using size_type = TSize;

    explicit SizedIterableFactory(TBegin begin, TEnd end, TSize size)
        : begin_{begin}
        , end_{end}
        , size_{size}
    {}
    TBegin begin() const {
        return begin_;
    }
    TEnd end() const {
        return end_;
    }
    TSize size() const {
        return size_;
    }
    bool empty() const {
        return size_ == 0;
    }
private:
    TBegin begin_;
    TEnd end_;
    TSize size_;
};

}
