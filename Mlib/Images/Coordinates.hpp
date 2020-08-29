#pragma once
#include <Mlib/Array/Array.hpp>
#include <cmath>
#include <cstddef>

namespace Mlib {

static const size_t id1 = 1;
static const size_t id0 = 0;

inline size_t a2i(float a) {
    return size_t(std::lround(std::floor(a)));
}

inline size_t fi2i(float fi) {
    return size_t(std::lround(std::round(fi)));
}

inline float i2fi(size_t i) {
    return float(i);
}

inline ArrayShape a2i(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{2}));
    return ArrayShape{
        a2i(a(id1)),
        a2i(a(id0))};
}

inline float i2a(size_t i) {
    // Adding 0.5 selects the center of the pixel
    return i + 0.5f;
}

inline Array<float> i2a(const ArrayShape& i) {
    assert(i.ndim() == 2);
    return Array<float>{
        i2a(i(id1)),
        i2a(i(id0))};
}

inline float a2fi(float a) {
    return a - 0.5f;
}

inline float fi2a(float fi) {
    return fi + 0.5f;
}

inline Array<float> a2fi(const Array<float>& a) {
    assert(all(a.shape() == ArrayShape{2}));
    return Array<float>{
        a2fi(a(id1)),
        a2fi(a(id0))};
}

}
