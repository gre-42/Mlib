#pragma once
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Stats/Random_Number_Generators.hpp>
#include <set>
#include <stdexcept>
#include <vector>

namespace Mlib {

static thread_local UniformIntRandomNumberGenerator<size_t> welzl_rng{43, 0, SIZE_MAX};

template <class TData, size_t tndim>
TData ssq(const FixedArray<TData, tndim>& x) {
    return sum(squared(x));
}

template <class TData, size_t tndim>
FixedArray<TData, tndim> cc_1(
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b,
    const FixedArray<TData, tndim>& c)
{
    return dot0d(a, c) * b - dot0d(a, b) * c;
}

template <class TData, size_t tndim>
TData cc_2(
    const FixedArray<TData, tndim>& a,
    const FixedArray<TData, tndim>& b)
{
    return sum(squared(a)) * sum(squared(b)) - squared(dot0d(a, b));
}

/** From: https://en.wikipedia.org/wiki/Circumscribed_circle#Higher_dimensions
 */
template <class TData, size_t tndim>
BoundingSphere<TData, tndim> circumscribed_sphere(const FixedArray<FixedArray<TData, tndim>, 3>& x)
{
    const FixedArray<TData, tndim>& A = x(0);
    const FixedArray<TData, tndim>& B = x(1);
    const FixedArray<TData, tndim>& C = x(2);
    FixedArray<TData, tndim> a = A - C;
    FixedArray<TData, tndim> b = B - C;
    TData denom = cc_2(a, b);
    if (std::abs(denom) < 1e-10) {
        return BoundingSphere<TData, tndim>{fixed_zeros<TData, tndim>(), TData(0)};
    }
    TData radius = std::sqrt(ssq(a) * ssq(b) * ssq(a - b) / denom) / TData(2);
    FixedArray<TData, tndim> center = cc_1(ssq(a) * b - ssq(b) * a, a, b) / (TData(2) * denom) + C;
    return BoundingSphere<TData, tndim>{center, radius};
}

/** From: https://en.wikipedia.org/wiki/Tetrahedron#Circumradius
 */
template <class TData>
BoundingSphere<TData, 3> circumscribed_sphere(const FixedArray<FixedArray<TData, 3>, 4>& x)
{
    FixedArray<TData, 3, 3> A;
    A[0] = x(1) - x(0);
    A[1] = x(2) - x(0);
    A[2] = x(3) - x(0);
    auto B = TData(0.5) * FixedArray<TData, 3>{ssq(A[0]), ssq(A[1]), ssq(A[2])};
    auto optional_center = lstsq_chol_1d<TData, 3, 3>(A, B, 0, 0, nullptr, nullptr, TData(1e-10));
    if (!optional_center.has_value()) {
        return BoundingSphere<TData, 3>{fixed_zeros<TData, 3>(), TData(0)};
    }
    auto center = optional_center.value() + x(0);
    return BoundingSphere<TData, 3>::from_center_and_iterator(center, x.flat_begin(), x.flat_end());
}

template <class TData, size_t tndim>
BoundingSphere<TData, tndim> circumscribed_sphere(const std::vector<const FixedArray<TData, tndim>*>& R) {
    if (R.size() == 0) {
        throw std::runtime_error("Cannot compute bounding sphere for empty list");
    }
    if (R.size() > tndim + 1) {
        throw std::runtime_error("Too many points to compute trivial circumscribed sphere");
    }
    if (R.size() == 1) {
        return BoundingSphere<TData, tndim>(*R[0], (TData)0);
    }
    if (R.size() == 2) {
        return BoundingSphere<TData, tndim>(FixedArray<FixedArray<TData, tndim>, 2>{*R[0], *R[1]});
    }
    if (R.size() == 3) {
        return circumscribed_sphere(FixedArray<FixedArray<TData, tndim>, 3>{*R[0], *R[1], *R[2]});
    }
    if constexpr (tndim == 3) {
        if (R.size() == 4) {
            return circumscribed_sphere(FixedArray<FixedArray<TData, 3>, 4>{*R[0], *R[1], *R[2], *R[3]});
        }
    }
    throw std::runtime_error("Cannot compute trivial bounding sphere for the specified dimension and number of points");
}

/** From: https://en.wikipedia.org/wiki/Smallest-circle_problem
 * input: Finite sets P and R of points in the plane |R| ≤ 3.
 * output: Minimal disk enclosing P with R on the boundary.
 */
template <class TData, size_t tndim, class TRng>
BoundingSphere<TData, tndim> welzl(
    const std::vector<const FixedArray<TData, tndim>*>& P,
    const std::vector<const FixedArray<TData, tndim>*>& R,
    TRng& rng)
{
    if (P.empty() || (R.size() == tndim + 1)) {
        return circumscribed_sphere(R);
    }
    size_t p = rng() % P.size();
    auto Pp = P;
    Pp.erase(Pp.begin() + p);
    if (!Pp.empty() || !R.empty()) {
        auto D = welzl(Pp, R, rng);
        if (D.contains(*P[p])) {
            return D;
        }
    }
    auto Rp = R;
    Rp.push_back(P[p]);
    return welzl(Pp, Rp, rng);
}

template <class TData, size_t tndim, class TRng>
BoundingSphere<TData, tndim> welzl_from_vector(
    const std::vector<const FixedArray<TData, tndim>*>& P,
    TRng& rng = welzl_rng)
{
    return welzl(P, {}, rng);
}

template <class TData, size_t tndim, size_t tnpoints, class TRng = decltype(welzl_rng)>
BoundingSphere<TData, tndim> welzl_from_fixed(
    const FixedArray<FixedArray<TData, tndim>, tnpoints>& P,
    TRng& rng = welzl_rng)
{
    std::vector<const FixedArray<TData, tndim>*> Pvec(tnpoints);
    for (size_t i = 0; i < tnpoints; ++i) {
        Pvec[i] = &P(i);
    }
    return welzl_from_vector(Pvec, rng);
}

template <class TData, size_t tndim, class TIterable, class TRng = decltype(welzl_rng)>
BoundingSphere<TData, tndim> welzl_from_iterator(
    const TIterable& P_begin,
    const TIterable& P_end,
    TRng& rng = welzl_rng)
{
    std::vector<const FixedArray<TData, tndim>*> Pvec;
    Pvec.reserve(P_end - P_begin);
    std::set<OrderableFixedArray<TData, tndim>> ptset;
    for (auto P_it = P_begin; P_it != P_end; ++P_it) {
        const auto& p = *P_it;
        if (ptset.insert(OrderableFixedArray{p}).second) {
            Pvec.push_back(&p);
        }
    }
    return welzl_from_vector(Pvec, rng);
}

}
