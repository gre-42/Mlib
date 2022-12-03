#pragma once
#include "Array_Forward.hpp"
#include <cassert>
#include <ostream>
#include <sstream>
#include <vector>

namespace Mlib {

inline std::ostream &operator << (std::ostream &ostream, const ArrayShape &v);

class ArrayShape {
    std::vector<size_t> shape_;
public:
    template <class Archive>
    void serialize(Archive& archive) {
        archive(shape_);
    }

    ArrayShape() = default;
    ArrayShape(const ArrayShape& shape) = default;
    ArrayShape(ArrayShape&& shape) = default;
    ArrayShape& operator = (const ArrayShape& shape) = default;
    ArrayShape& operator = (ArrayShape&& shape) = default;
    explicit ArrayShape(std::initializer_list<size_t> shape) {
        for (size_t s : shape) {
            append(s);
        }
    }
    explicit ArrayShape(size_t ndim):
        shape_(ndim)
    {}
    ArrayShape& operator = (size_t value) {
        for (size_t d = 0; d < ndim(); ++d) {
            shape_[d] = value;
        }
        return *this;
    }
    void append(size_t size) {
        shape_.push_back(size);
    }
    ArrayShape appended(size_t size) const {
        ArrayShape result = *this;
        result.append(size);
        return result;
    }
    inline ArrayShape erased_first(size_t n = 1) const {
        assert(ndim() > 0);
        assert(n <= ndim());
        ArrayShape result = *this;
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
        result.shape_.erase(result.shape_.begin(), result.shape_.begin() + n);
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif
        return result;
    }
    ArrayShape erased_last(size_t n = 1) const {
        assert(ndim() > 0);
        assert(n <= ndim());
        ArrayShape result = *this;
        result.shape_.erase(result.shape_.end() - n, result.shape_.end());
        return result;
    }
    void clear() {
        shape_.clear();
    }
    ArrayShape reverted() const {
        ArrayShape result;
        for (size_t i = 0; i < ndim(); i++) {
            result.append((*this)(ndim() - i - 1));
        }
        return result;
    }
    void concatenate(const ArrayShape& other) {
        for (size_t i = 0; i < other.ndim(); i++) {
            append(other(i));
        }
    }
    ArrayShape concatenated(const ArrayShape& other) const {
        ArrayShape result = *this;
        result.concatenate(other);
        return result;
    }
    const size_t& operator () (size_t i) const {
        assert(i < ndim());
        return shape_[i];
    }
    size_t& operator () (size_t i) {
        const ArrayShape& a = *this;
        return const_cast<size_t&>(a(i));
    }
    template <size_t N>
    const size_t& get() const {
        return (*this)(N);
    }
    size_t ndim() const {
        return shape_.size();
    }
    size_t nelements() const {
        size_t result = 1;
        for (size_t i=0; i<ndim(); i++) {
            result *= (*this)(i);
        }
        return result;
    }
    size_t length() const {
        assert(ndim() == 1);
        return (*this)(0);
    }
    template <class TCallable>
    void foreach(
        const TCallable& f,
        const ArrayShape& prepend=ArrayShape()) const
    {
        if (ndim() == 0) {
            f(prepend);
        } else {
            // original code
            // for (size_t i=0; i<(*this)(0); i++) {
            //    erased_first().foreach(f, prepend.appended(i));
            //}
            // optimized code
            ArrayShape ef = erased_first();
            ArrayShape id = prepend.appended(SIZE_MAX);
            size_t last_id = id.ndim() - 1;
            for (size_t i = 0; i < (*this)(0); i++) {
                id(last_id) = i;
                ef.foreach(f, id);
            }
        }
    }
    template <class TCallable>
    void apply_over_axis(
        size_t axis,
        const TCallable& f) const
    {
        assert(axis < ndim());
        ArrayShape shape;
        shape = *this;
        shape(axis) = 1;
        shape.foreach([&](const ArrayShape& index0){
            f(index0);
        });
    }
    std::string str() const {
        std::stringstream sstr;
        sstr << *this;
        return sstr.str();
    }
    size_t* begin() {
        return shape_.data();
    }
    const size_t* begin() const {
        return shape_.data();
    }
};

template <class TBinop>
inline ArrayShape arrayshape_arrayshape_binop(const ArrayShape& a, const ArrayShape& b, const TBinop& binop) {
    assert(a.ndim() == b.ndim());
    ArrayShape result(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = binop(a(i), b(i));
    }
    return result;
}

template <class TOperation>
inline ArrayShape arrayshape_apply(const ArrayShape& a, const TOperation& operation) {
    ArrayShape result(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = operation(a(i));
    }
    return result;
}

inline ArrayShape operator + (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_binop(a, b, [](size_t x, size_t y){ return x + y; });
}

inline ArrayShape operator - (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_binop(a, b, [](size_t x, size_t y){ return x - y; });
}

inline ArrayShape operator / (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_binop(a, b, [&](size_t x, size_t y){ return x / y; });
}

inline ArrayShape operator % (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_binop(a, b, [&](size_t x, size_t y){ return x % y; });
}

inline ArrayShape operator + (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x + b; });
}

inline ArrayShape operator - (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x - b; });
}

inline ArrayShape operator * (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x * b; });
}

inline ArrayShape operator * (size_t a, const ArrayShape& b) {
    return b * a;
}

inline ArrayShape operator / (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x / b; });
}

inline ArrayShape operator % (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x % b; });
}

inline ArrayShape operator & (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x & b; });
}

inline ArrayShape operator | (const ArrayShape& a, size_t b) {
    return arrayshape_apply(a, [&](size_t x){ return x | b; });
}

inline std::ostream &operator << (std::ostream &ostream, const ArrayShape &v) {
    for (size_t i=0; i<v.ndim(); i++) {
        ostream << v(i);
        if (i != v.ndim() - 1) {
            ostream << " ";
        }
    }
    return ostream;
}

}
