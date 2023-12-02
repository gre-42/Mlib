#pragma once

namespace Mlib {

template <typename TData>
class PointerIterable {
public:
    PointerIterable(TData* begin, TData* end)
        : begin_(begin)
        , end_(end)
    {}
    TData* begin() {
        return begin_;
    }
    TData* end() {
        return end_;
    }
private:
    TData* begin_;
    TData* end_;
};

}
