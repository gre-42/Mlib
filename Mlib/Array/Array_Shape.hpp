#pragma once
#include "Array_Forward.hpp"
#include <cassert>
#include <cstdint>
#include <iosfwd>
#include <stdexcept>
#include <string>
#include <vector>

namespace Mlib {

std::ostream &operator << (std::ostream &ostream, const ArrayShape &v);

template <class TCallable>
void foreach(const ArrayShape& begin, const ArrayShape& end, const TCallable& f);

class ArrayShape {
    std::vector<size_t> shape_;
    using iter_diff_type = std::vector<size_t>::iterator::difference_type;
public:
    template <class Archive>
    void serialize(Archive& archive) {
        archive(shape_);
    }

    ArrayShape();
    ArrayShape(const ArrayShape& shape);
    ArrayShape(ArrayShape&& shape);
    ArrayShape& operator = (const ArrayShape& shape);
    explicit ArrayShape(std::initializer_list<size_t> shape);
    explicit ArrayShape(size_t ndim);
    ArrayShape& operator = (size_t value);
    void append(size_t size);
    ArrayShape appended(size_t size) const;
    ArrayShape erased_first(size_t n = 1) const;
    ArrayShape erased_last(size_t n = 1) const;
    void clear();
    ArrayShape reverted() const;
    void concatenate(const ArrayShape& other);
    ArrayShape concatenated(const ArrayShape& other) const;
    inline const size_t& operator () (size_t i) const {
        assert(i < ndim());
        return shape_[i];
    }
    inline size_t& operator () (size_t i) {
        const ArrayShape& a = *this;
        return const_cast<size_t&>(a(i));
    }
    template <size_t N>
    const size_t& get() const {
        return (*this)(N);
    }
    size_t ndim() const;
    size_t nelements() const;
    size_t length() const;
    template <class TCallable>
    void foreach(const TCallable& f) const {
        ArrayShape begin(ndim());
        begin = 0;
        Mlib::foreach(begin, *this, f);
    }
    template <class TCallable>
    void apply_over_axis(
        size_t axis,
        const TCallable& f) const
    {
        assert(axis < ndim());
        ArrayShape shape(ndim());
        shape = *this;
        shape(axis) = 1;
        shape.foreach([&](const ArrayShape& index0){
            f(index0);
        });
    }
    std::string str() const;
    size_t* begin();
    const size_t* begin() const;
};

template <class TCallable>
void foreach(const ArrayShape& begin, const ArrayShape& end, const TCallable& f)
{
    assert(begin.ndim() == end.ndim());
    ArrayShape index(begin.ndim());
    switch (begin.ndim()) {
    case 0:
        f(index);
        break;
    case 1:
        for (index(0) = begin(0); index(0) < end(0); ++index(0)) {
            f(index);
        }
        break;
    case 2:
        for (index(0) = begin(0); index(0) < end(0); ++index(0)) {
            for (index(1) = begin(1); index(1) < end(1); ++index(1)) {
                f(index);
            }
        }
        break;
    case 3:
        for (index(0) = begin(0); index(0) < end(0); ++index(0)) {
            for (index(1) = begin(1); index(1) < end(1); ++index(1)) {
                for (index(2) = begin(2); index(2) < end(2); ++index(2)) {
                    f(index);
                }
            }
        }
        break;
    case 4:
        for (index(0) = begin(0); index(0) < end(0); ++index(0)) {
            for (index(1) = begin(1); index(1) < end(1); ++index(1)) {
                for (index(2) = begin(2); index(2) < end(2); ++index(2)) {
                    for (index(3) = begin(3); index(3) < end(3); ++index(3)) {
                        f(index);
                    }
                }
            }
        }
        break;
    default:
        throw std::runtime_error("foreach: Shape dimensionality too large");
    }
}

template <class TBinop>
inline void arrayshape_arrayshape_apply(ArrayShape& a, const ArrayShape& b, const TBinop& binop) {
    assert(a.ndim() == b.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        binop(a(i), b(i));
    }
}

template <class TBinop>
inline ArrayShape arrayshape_arrayshape_applied(const ArrayShape& a, const ArrayShape& b, const TBinop& binop) {
    assert(a.ndim() == b.ndim());
    ArrayShape result(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = binop(a(i), b(i));
    }
    return result;
}

template <class TOperation>
inline ArrayShape arrayshape_applied(const ArrayShape& a, const TOperation& operation) {
    ArrayShape result(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = operation(a(i));
    }
    return result;
}

inline ArrayShape& operator += (ArrayShape& a, const ArrayShape& b) {
    arrayshape_arrayshape_apply(a, b, [](size_t& x, size_t y){ return x += y; });
    return a;
}

inline ArrayShape& operator -= (ArrayShape& a, const ArrayShape& b) {
    arrayshape_arrayshape_apply(a, b, [](size_t& x, size_t y){ return x -= y; });
    return a;
}

inline ArrayShape operator + (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_applied(a, b, [](size_t x, size_t y){ return x + y; });
}

inline ArrayShape operator - (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_applied(a, b, [](size_t x, size_t y){ return x - y; });
}

inline ArrayShape operator / (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_applied(a, b, [&](size_t x, size_t y){ return x / y; });
}

inline ArrayShape operator % (const ArrayShape& a, const ArrayShape& b) {
    return arrayshape_arrayshape_applied(a, b, [&](size_t x, size_t y){ return x % y; });
}

inline ArrayShape operator + (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x + b; });
}

inline ArrayShape operator - (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x - b; });
}

inline ArrayShape operator * (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x * b; });
}

inline ArrayShape operator * (size_t a, const ArrayShape& b) {
    return b * a;
}

inline ArrayShape operator / (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x / b; });
}

inline ArrayShape operator % (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x % b; });
}

inline ArrayShape operator & (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x & b; });
}

inline ArrayShape operator | (const ArrayShape& a, size_t b) {
    return arrayshape_applied(a, [&](size_t x){ return x | b; });
}

}
