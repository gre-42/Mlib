#pragma once
#include "Array.hpp"
#include "Array_Shape.hpp"
#include <map>
#include <memory>

namespace Mlib {

template <class TData>
class SparseArrayCcs {
public:
    typedef TData value_type;

    SparseArrayCcs() {}

    explicit SparseArrayCcs(const ArrayShape& shape) {
        assert(shape.ndim() == 2);
        data_ = std::make_shared<std::vector<std::map<size_t, TData>>>(shape(1));
        shape_ = std::make_shared<ArrayShape>(shape);
    }

    explicit SparseArrayCcs(const Array<TData>& rhs)
    : SparseArrayCcs{rhs.shape()}
    {
        assert(rhs.ndim() == 2);
        for (size_t r = 0; r < rhs.shape(0); ++r) {
            for (size_t c = 0; c < rhs.shape(1); ++c) {
                if (rhs(r, c) != TData(0)) {
                    (*this)(r, c) = rhs(r, c);
                }
            }
        }
    }

    void resize(const ArrayShape& shape) {
        assert(shape.ndim() == 2);
        if (shape(0) < (*shape_)(0)) {
            throw std::runtime_error("Cannot reduce the number of rows in SparseArrayCcs");
        }
        data_->resize(shape(1));
        *shape_ = shape;
    }

    SparseArrayCcs copy() const {
        SparseArrayCcs result{shape()};
        *result.data_ = *data_;
        return result;
    }

    const ArrayShape& shape() const {
        assert(shape_ != nullptr);
        return *shape_;
    }

    size_t shape(size_t i) const {
        return shape()(i);
    }

    size_t ndim() const {
        return shape().ndim();
    }

    SparseArrayCcs vH() const {
        SparseArrayCcs result{*this};
        result.conjugated_transposed = !conjugated_transposed;
        return result;
    }

    std::vector<std::map<size_t, TData>>& columns() {
        assert(data_ != nullptr);
        return *data_;
    }

    std::map<size_t, TData>& column(size_t c) {
        assert(c < columns().size());
        return columns()[c];
    }

    TData& operator () (size_t r, size_t c) {
        assert(r < shape(0));
        assert(c < shape(1));
        assert(!conjugated_transposed);
        return column(c)[r];
    }

    const std::vector<std::map<size_t, TData>>& columns() const {
        return const_cast<SparseArrayCcs*>(this)->columns();
    }

    const std::map<size_t, TData>& column(size_t c) const {
        return const_cast<SparseArrayCcs*>(this)->column(c);
    }

    SparseArrayCcs columns(const Array<size_t>& column_ids) const {
        SparseArrayCcs<float> res{ArrayShape{shape(0), column_ids.length()}};
        for (size_t i = 0; i < column_ids.length(); ++i) {
            res.column(i) = column(column_ids(i));
        }
        return res;
    }

    const TData& operator () (size_t r, size_t c) const {
        return (*const_cast<SparseArrayCcs*>(this))(r, c);
    }

    Array<TData> to_dense_array() const {
        assert(!conjugated_transposed);
        Array<TData> res{shape()};
        for (size_t r = 0; r < shape(0); ++r) {
            for (size_t c = 0; c < shape(1); ++c) {
                res(r, c) = (*this)(r, c);
            }
        }
        return res;
    }

    template <class TResultData = TData, class TOperation>
    SparseArrayCcs<TResultData> apply_to_defined(const TOperation& op) const {
        SparseArrayCcs<TResultData> result{shape()};
        for (size_t c = 0; c < shape(1); ++c) {
            for (const auto& d : column(c)) {
                result(d.first, c) = op(d.second);
            }
        }
        return result;
    }

    template <class TResultData>
    SparseArrayCcs<TResultData> casted() const {
        return apply_to_defined<TResultData>([](const TData& v){ return v; });
    }

    SparseArrayCcs<bool> is_defined() const {
        return apply_to_defined<bool>([](const TData& v){ return true; });
    }

    Array<bool> row_is_defined() const {
        Array<bool> result = full(ArrayShape{shape(0)}, false);
        for (const auto& c : columns()) {
            for (const auto& d : c) {
                result(d.first) = true;
            }
        }
        return result;
    }

    bool initialized() const {
        return shape_ != nullptr;
    }

    bool conjugated_transposed = false;
private:
    std::shared_ptr<std::vector<std::map<size_t, TData>>> data_;
    std::shared_ptr<ArrayShape> shape_;

};

template <class TData>
Array<TData> operator , (const SparseArrayCcs<TData>& a, const SparseArrayCcs<TData>& b) {
    throw std::runtime_error("Sparse: please use outer or dot");
}

template <class TData>
Array<TData> dot2d(const SparseArrayCcs<TData>& a, const SparseArrayCcs<TData>& b) {
    assert(a.conjugated_transposed);
    assert(!b.conjugated_transposed);
    assert(a.ndim() == 2);
    assert(b.ndim() == 2);
    assert(a.shape(0) == b.shape(0));
    Array<TData> result{ArrayShape{a.shape(1), b.shape(1)}};
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            const auto& col_a = a.column(r);
            const auto& col_b = b.column(c);
            TData v = 0;
            auto it_a = col_a.begin();
            auto it_b = col_b.begin();
            while(
                it_a != col_a.end() &&
                it_b != col_b.end())
            {
                if (it_a->first == it_b->first) {
                    v += conju(it_a->second) * it_b->second;
                    ++it_a;
                    ++it_b;
                } else if (it_a->first < it_b->first) {
                    ++it_a;
                } else if (it_b->first < it_a->first) {
                    ++it_b;
                }
            }
            result(r, c) = v;
        }
    }
    return result;
}

template <class TData>
Array<TData> operator , (const SparseArrayCcs<TData>& a, const Array<TData>& b) {
    throw std::runtime_error("Sparse: please use outer or dot");
}

template <class TData>
Array<TData> dot2d(const SparseArrayCcs<TData>& a, const Array<TData>& b) {
    assert(a.conjugated_transposed);
    assert(a.ndim() == 2);
    assert(b.ndim() == 2);
    assert(a.shape(0) == b.shape(0));
    Array<TData> result{ArrayShape{a.shape(1), b.shape(1)}};
    for (size_t r = 0; r < result.shape(0); ++r) {
        for (size_t c = 0; c < result.shape(1); ++c) {
            const auto& col_a = a.column(r);
            TData v = 0;
            for (const auto& ea : col_a) {
                v += conju(ea.second) * b(ea.first, c);
            }
            result(r, c) = v;
        }
    }
    return result;
}

template <class TData>
Array<TData> dot1d(const SparseArrayCcs<TData>& a, const Array<TData>& b) {
    assert(b.ndim() == 1);
    return dot2d(a, b.reshaped(ArrayShape{b.length(), 1})).flattened();
}

template <class TData>
std::ostream& operator << (std::ostream& ostr, const SparseArrayCcs<TData>& ar) {
    for (size_t c = 0; c < ar.shape(1); ++c) {
        ostr << "c " << c << ":";
        for (const auto& v : ar.column(c)) {
            ostr << " (" << v.first << ", " << v.second << ")";
        }
        ostr << std::endl;
    }
    return ostr;
}

template <class TData>
SparseArrayCcs<TData> operator * (const SparseArrayCcs<TData>& a, const TData& b) {
    return a.apply_to_defined([&b](const TData& x){ return x * b; });
}

template <class TData>
SparseArrayCcs<TData> operator / (const SparseArrayCcs<TData>& a, const TData& b) {
    return a.apply_to_defined([&b](const TData& x){ return x / b; });
}

}
