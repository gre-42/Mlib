#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Fixed_Array.hpp>

namespace Mlib {

template<class TData, size_t tdata_dimension>
class VectorialPixels: public Array<FixedArray<TData, tdata_dimension>> {
public:

    VectorialPixels(const ArrayShape& shape)
    : Array<FixedArray<TData, tdata_dimension>>{shape}
    {}

    Array<TData> to_array() const {
        Array<TData> result{ArrayShape{tdata_dimension}.concatenated(this->shape())};
        for(size_t d = 0; d < tdata_dimension; ++d) {
            result[d] = this->template applied<TData>([&](const auto& p){ return p(d); });
        }
        return result;
    }

};

};
