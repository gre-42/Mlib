#pragma once
#include "Array.hpp"
#include "Array_Shape.hpp"
#include <Mlib/Array/Object_Vector.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <map>
#include <memory>
#include <stdexcept>

namespace Mlib {

template <class TData, class tsize>
class SparseArrayCcs {
public:
    using value_type = TData;
    using Size = tsize;

    SparseArrayCcs() {}

    explicit SparseArrayCcs(const ArrayShape& shape) {
        assert(shape.ndim() == 2);
        data_ = std::make_shared<ObjectVector<std::map<tsize, TData>>>(shape(1));
        shape_ = std::make_shared<ArrayShape>(shape);
    }

    explicit SparseArrayCcs(tsize r, tsize c)
        : SparseArrayCcs{ArrayShape{r, c}}
    {}

    explicit SparseArrayCcs(const Array<TData>& rhs)
        : SparseArrayCcs{rhs.shape()}
    {
        assert(rhs.ndim() == 2);
        for (tsize r = 0; r < rhs.shape(0); ++r) {
            for (tsize c = 0; c < rhs.shape(1); ++c) {
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

    void resize(tsize r, tsize c) {
        resize(ArrayShape{r, c});
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

    tsize shape(tsize i) const {
        return shape()(i);
    }

    tsize ndim() const {
        return shape().ndim();
    }

    SparseArrayCcs vH() const {
        SparseArrayCcs result{*this};
        result.conjugated_transposed = !conjugated_transposed;
        return result;
    }

    std::vector<std::map<tsize, TData>>& columns() {
        assert(data_ != nullptr);
        return *data_;
    }

    std::map<tsize, TData>& column(tsize c) {
        assert(c < columns().size());
        return columns()[c];
    }

    TData& operator () (tsize r, tsize c) {
        assert(r < shape(0));
        assert(c < shape(1));
        assert(!conjugated_transposed);
        return column(c)[r];
    }

    const std::vector<std::map<tsize, TData>>& columns() const {
        return const_cast<SparseArrayCcs*>(this)->columns();
    }

    const std::map<tsize, TData>& column(tsize c) const {
        return const_cast<SparseArrayCcs*>(this)->column(c);
    }

    SparseArrayCcs columns(const Array<tsize>& column_ids) const {
        SparseArrayCcs<float, tsize> res{ArrayShape{shape(0), column_ids.length()}};
        for (tsize i = 0; i < column_ids.length(); ++i) {
            res.column(i) = column(column_ids(i));
        }
        return res;
    }

    const TData& operator () (tsize r, tsize c) const {
        return (*const_cast<SparseArrayCcs*>(this))(r, c);
    }

    Array<TData> to_dense_array() const {
        assert(!conjugated_transposed);
        Array<TData> res{shape()};
        for (tsize r = 0; r < shape(0); ++r) {
            for (tsize c = 0; c < shape(1); ++c) {
                res(r, c) = (*this)(r, c);
            }
        }
        return res;
    }

    // Apply an operation to all defined (e.g. non-zero or finite, depending on the application) entries
    template <class TResultData = TData, class TOperation>
    SparseArrayCcs<TResultData, tsize> applied_to_defined(const TOperation& op) const {
        SparseArrayCcs<TResultData, tsize> result{shape()};
        for (tsize c = 0; c < shape(1); ++c) {
            for (const auto& [r, value] : column(c)) {
                result(r, c) = op(value);
            }
        }
        return result;
    }

    // Apply an operation to all defined (e.g. non-zero or finite, depending on the application) entries
    template <class TOperation>
    void apply_to_defined(const TOperation& op) {
        for (tsize c = 0; c < shape(1); ++c) {
            for (auto& [_, value] : column(c)) {
                op(value);
            }
        }
    }

    template <class TResultData>
    SparseArrayCcs<TResultData, tsize> casted() const {
        return applied_to_defined<TResultData>([](const TData& v){ return v; });
    }

    SparseArrayCcs<bool, tsize> is_defined() const {
        return applied_to_defined<bool>([](const TData& v){ return true; });
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

    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        if constexpr (Archive::is_saving::value) {
            archive(conjugated_transposed);
            archive(data_);
            archive(shape_);
        } else {
            SparseArrayCcs tmp;
            archive(tmp.conjugated_transposed);
            archive(tmp.data_);
            archive(tmp.shape_);
            if (tmp.data_ == nullptr) {
                throw std::runtime_error("Sparse array has no data");
            }
            if (tmp.shape_ == nullptr) {
                throw std::runtime_error("Sparse array has no shape");
            }
            if (tmp.shape_->ndim() != 2) {
                throw std::runtime_error("Loaded array does not have 2 dimensions");
            }
            if (tmp.shape(1) != tmp.data_->size()) {
                throw std::runtime_error("Loaded array has inconsistent number of columns");
            }
            for (const auto& c : tmp.columns()) {
                for (const auto& r : c) {
                    if (r.first >= tmp.shape(0)) {
                        throw std::runtime_error("Loaded array has inconsistent number of rows");
                    }
                }
            }
            conjugated_transposed = tmp.conjugated_transposed;
            data_ = std::move(tmp.data_);
            shape_ = std::move(tmp.shape_);
        }
    }

    bool conjugated_transposed = false;
private:
    std::shared_ptr<ObjectVector<std::map<tsize, TData>>> data_;
    std::shared_ptr<ArrayShape> shape_;

};

template <class TData, class tsize>
Array<TData> operator , (const SparseArrayCcs<TData, tsize>& a, const SparseArrayCcs<TData, tsize>& b) {
    throw std::runtime_error("Sparse: please use outer or dot");
}

template <class TData, class tsize>
Array<TData> dot2d(const SparseArrayCcs<TData, tsize>& a, const SparseArrayCcs<TData, tsize>& b) {
    assert(a.conjugated_transposed);
    assert(!b.conjugated_transposed);
    assert(a.ndim() == 2);
    assert(b.ndim() == 2);
    assert(a.shape(0) == b.shape(0));
    Array<TData> result{ArrayShape{a.shape(1), b.shape(1)}};
    for (tsize r = 0; r < result.shape(0); ++r) {
        for (tsize c = 0; c < result.shape(1); ++c) {
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

template <class TData, class tsize>
Array<TData> operator , (const SparseArrayCcs<TData, tsize>& a, const Array<TData>& b) {
    throw std::runtime_error("Sparse: please use outer or dot");
}

template <class TData, class tsize>
Array<TData> dot2d(const SparseArrayCcs<TData, tsize>& a, const Array<TData>& b) {
    assert(a.conjugated_transposed);
    assert(a.ndim() == 2);
    assert(b.ndim() == 2);
    assert(a.shape(0) == b.shape(0));
    Array<TData> result{ArrayShape{a.shape(1), b.shape(1)}};
    for (tsize r = 0; r < result.shape(0); ++r) {
        for (tsize c = 0; c < result.shape(1); ++c) {
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

template <class TData, class tsize>
Array<TData> dot1d(const SparseArrayCcs<TData, tsize>& a, const Array<TData>& b) {
    assert(b.ndim() == 1);
    return dot2d(a, b.reshaped(ArrayShape{b.length(), 1})).flattened();
}

template <class TData, class tsize>
std::ostream& operator << (std::ostream& ostr, const SparseArrayCcs<TData, tsize>& ar) {
    for (tsize c = 0; c < ar.shape(1); ++c) {
        ostr << "c " << c << ":";
        for (const auto& v : ar.column(c)) {
            ostr << " (" << v.first << ", " << v.second << ")";
        }
        ostr << std::endl;
    }
    return ostr;
}

template <class TData, class TOther, class tsize>
SparseArrayCcs<TData, tsize>& operator *= (SparseArrayCcs<TData, tsize>& a, const TOther& b) {
    a.apply_to_defined([&b](TData& x){ x *= b; });
    return a;
}

template <class TData, class TOther, class tsize>
SparseArrayCcs<TData, tsize>& operator /= (SparseArrayCcs<TData, tsize>& a, const TOther& b) {
    a.apply_to_defined([&b](TData& x){ x /= b; });
    return a;
}

template <class TData, class TOther, class tsize>
SparseArrayCcs<TData, tsize> operator * (const SparseArrayCcs<TData, tsize>& a, const TOther& b) {
    return a.applied_to_defined([&b](const TData& x){ return x * b; });
}

template <class TData, class TOther, class tsize>
SparseArrayCcs<TData, tsize> operator / (const SparseArrayCcs<TData, tsize>& a, const TOther& b) {
    return a.applied_to_defined([&b](const TData& x){ return x / b; });
}

}
