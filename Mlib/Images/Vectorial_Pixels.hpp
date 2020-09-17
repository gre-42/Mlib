#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template<class TData, size_t tdata_dimension>
class VectorialPixels: public Array<FixedArray<TData, tdata_dimension>> {
public:

    explicit VectorialPixels(const ArrayShape& shape)
    : Array<FixedArray<TData, tdata_dimension>>{shape}
    {}

    explicit VectorialPixels(const Array<TData>& ar)
    : Array<FixedArray<TData, tdata_dimension>>{ar.shape().erased_first()}
    {
        assert(ar.shape(0) == tdata_dimension);
        for(size_t d = 0; d < tdata_dimension; ++d) {
            auto a_fi = ar[d].flat_iterable();
            auto fi = this->flat_iterable();
            auto a_it = a_fi.begin();
            auto it = fi.begin();
            while(a_it != a_fi.end()) {
                (*it)(d) = *a_it;
                ++a_it;
                ++it;
            }
        }
    }

    Array<TData> to_array() const {
        Array<TData> result{ArrayShape{tdata_dimension}.concatenated(this->shape())};
        for(size_t d = 0; d < tdata_dimension; ++d) {
            result[d] = this->template applied<TData>([&](const auto& p){ return p(d); });
        }
        return result;
    }

};

};
