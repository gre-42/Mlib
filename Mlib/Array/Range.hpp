#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class XRange {
public:
    inline XRange(const TData& begin, const TData& end)
    : begin(begin),
      end(end)
    {}
    inline TData length() const {
        return end - begin;
    }
    TData begin;
    TData end;
};

typedef XRange<size_t> Range;

}
