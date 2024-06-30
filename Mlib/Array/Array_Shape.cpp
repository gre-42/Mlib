#include "Array_Shape.hpp"
#include <sstream>

using namespace Mlib;

ArrayShape::ArrayShape() = default;
ArrayShape::ArrayShape(const ArrayShape& shape) = default;
ArrayShape::ArrayShape(ArrayShape&& shape) = default;
ArrayShape& ArrayShape::operator = (const ArrayShape& shape) = default;
ArrayShape& ArrayShape::operator = (ArrayShape&& shape) = default;
ArrayShape::ArrayShape(std::initializer_list<size_t> shape) {
    shape_.reserve(shape.size());
    for (size_t s : shape) {
        append(s);
    }
}
ArrayShape::ArrayShape(size_t ndim):
    shape_(ndim)
{}
ArrayShape& ArrayShape::operator = (size_t value) {
    for (size_t d = 0; d < ndim(); ++d) {
        shape_[d] = value;
    }
    return *this;
}
void ArrayShape::append(size_t size) {
    shape_.push_back(size);
}
ArrayShape ArrayShape::appended(size_t size) const {
    ArrayShape result = *this;
    result.append(size);
    return result;
}
ArrayShape ArrayShape::erased_first(size_t n) const {
    assert(ndim() > 0);
    assert(n <= ndim());
    ArrayShape result = *this;
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif
    result.shape_.erase(result.shape_.begin(), result.shape_.begin() + (iter_diff_type)n);
#if defined(__GNUC__) && !defined(__llvm__) && !defined(__INTEL_COMPILER) && !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif
    return result;
}
ArrayShape ArrayShape::erased_last(size_t n) const {
    assert(ndim() > 0);
    assert(n <= ndim());
    ArrayShape result = *this;
    result.shape_.erase(result.shape_.end() - (iter_diff_type)n, result.shape_.end());
    return result;
}
void ArrayShape::clear() {
    shape_.clear();
}
ArrayShape ArrayShape::reverted() const {
    ArrayShape result;
    for (size_t i = 0; i < ndim(); i++) {
        result.append((*this)(ndim() - i - 1));
    }
    return result;
}
void ArrayShape::concatenate(const ArrayShape& other) {
    for (size_t i = 0; i < other.ndim(); i++) {
        append(other(i));
    }
}
ArrayShape ArrayShape::concatenated(const ArrayShape& other) const {
    ArrayShape result = *this;
    result.concatenate(other);
    return result;
}
const size_t& ArrayShape::operator () (size_t i) const {
    assert(i < ndim());
    return shape_[i];
}
size_t& ArrayShape::operator () (size_t i) {
    const ArrayShape& a = *this;
    return const_cast<size_t&>(a(i));
}
size_t ArrayShape::ndim() const {
    return shape_.size();
}
size_t ArrayShape::nelements() const {
    size_t result = 1;
    for (size_t i=0; i<ndim(); i++) {
        result *= (*this)(i);
    }
    return result;
}
size_t ArrayShape::length() const {
    assert(ndim() == 1);
    return (*this)(0);
}

std::string ArrayShape::str() const {
    std::stringstream sstr;
    sstr << *this;
    return sstr.str();
}
size_t* ArrayShape::begin() {
    return shape_.data();
}
const size_t* ArrayShape::begin() const {
    return shape_.data();
}

std::ostream& Mlib::operator << (std::ostream &ostream, const ArrayShape &v) {
    for (size_t i=0; i<v.ndim(); i++) {
        ostream << v(i);
        if (i != v.ndim() - 1) {
            ostream << " ";
        }
    }
    return ostream;
}
