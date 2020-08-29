#pragma once
#include "Array_Forward.hpp"
#include <Mlib/Math/Conju.hpp>

namespace Mlib {

template <class TData>
class ConjugateTransposeArray: public BaseDenseArray<ConjugateTransposeArray<TData>, TData> {
public:
    explicit ConjugateTransposeArray(const Array<TData>& a)
    : a_{a},
      shape_{a.shape(1), a.shape(0)}
    {
        assert(a.ndim() == 2);
    }
    const ArrayShape& shape() const {
        return shape_;
    }
    size_t shape(size_t i) const {
        return shape_(i);
    }
    size_t ndim() const {
        return 2;
    }
    TData operator () (size_t r, size_t c) const {
        return conju(a_(c, r));
    }
private:
    const Array<TData>& a_;
    const ArrayShape shape_;
};

}
