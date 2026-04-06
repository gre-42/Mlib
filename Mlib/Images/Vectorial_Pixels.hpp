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

    static VectorialPixels from_array(const Array<TData>& ar) {
        auto result = VectorialPixels(ar.shape().erased_first());
        assert(ar.shape(0) == tdata_dimension);
        for (size_t d = 0; d < tdata_dimension; ++d) {
            auto a_fi = ar[d].flat_iterable();
            auto fi = result.flat_iterable();
            auto a_it = a_fi.begin();
            auto it = fi.begin();
            while(a_it != a_fi.end()) {
                (*it)(d) = *a_it;
                ++a_it;
                ++it;
            }
        }
        return result;
    }

    static VectorialPixels from_vector(const Array<TData>& ar) {
        if (ar.ndim() < 1) {
            throw std::runtime_error("VectorialPixels dimensionality too low");
        }
        if (ar.shape(ar.ndim() - 1) != tdata_dimension) {
            throw std::runtime_error("VectorialPixels data dimension incorrect");
        }
        static_assert(sizeof(FixedArray<TData, tdata_dimension>) == tdata_dimension * sizeof(TData));
        auto result = VectorialPixels(ar.shape().erased_last());
        std::copy(ar.flat_begin(), ar.flat_end(), (TData*)result.flat_begin());
        return result;
    }

    Array<TData> to_array() const {
        Array<TData> result{ArrayShape{tdata_dimension}.concatenated(this->shape())};
        for (size_t d = 0; d < tdata_dimension; ++d) {
            result[d] = this->template applied<TData>([&](const auto& p){ return p(d); });
        }
        return result;
    }

};

};
