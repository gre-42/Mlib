#pragma once
#include <cstddef>

namespace Mlib {

class Range {
public:
    inline Range(size_t begin, size_t end)
    : begin(begin),
      end(end)
    {}
    inline size_t length() const {
        return end - begin;
    }
    size_t begin;
    size_t end;
};

}
