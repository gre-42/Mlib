#pragma once
#include <Mlib/Geometry/Intersection/Bounding_Sphere.hpp>
#include <Mlib/Math/Fixed_Cholesky.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <optional>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace Mlib {

inline std::minstd_rand welzl_rng() {
    std::minstd_rand e;
    std::ignore = e();
    return e;
}

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
    return ssq(a) * ssq(b) - squared(dot0d(a, b));
}

/** From: https://en.wikipedia.org/wiki/Circumcircle#Higher_dimensions
 */
template <class TData, size_t tndim, class TRng>
std::optional<BoundingSphere<TData, tndim>> circumscribed_sphere(
    const FixedArray<FixedArray<TData, tndim>, 3>& x,
    TRng& rng)
{
    const FixedArray<TData, tndim>& A = x(0);
    const FixedArray<TData, tndim>& B = x(1);
    const FixedArray<TData, tndim>& C = x(2);
    FixedArray<TData, tndim> a = A - C;
    FixedArray<TData, tndim> b = B - C;
    TData denom = cc_2(a, b);
    if (denom < 1e-10) {
        return std::nullopt;
    }
    TData radius = std::sqrt(ssq(a) * ssq(b) * ssq(a - b) / denom) / TData(2);
    FixedArray<TData, tndim> center = cc_1(ssq(a) * b - ssq(b) * a, a, b) / (TData(2) * denom) + C;
    return BoundingSphere<TData, tndim>{ center, radius };
}

/** From: https://en.wikipedia.org/wiki/Tetrahedron#Circumradius
 */
template <class TData, class TRng>
std::optional<BoundingSphere<TData, 3>> circumscribed_sphere(
    const FixedArray<FixedArray<TData, 3>, 4>& x,
    TRng& rng)
{
    FixedArray<TData, 3, 3> A;
    A[0] = x(1) - x(0);
    A[1] = x(2) - x(0);
    A[2] = x(3) - x(0);
    auto B = TData(0.5) * FixedArray<TData, 3>{ssq(A[0]), ssq(A[1]), ssq(A[2])};
    auto optional_center = lstsq_chol_1d<TData, 3, 3>(A, B, 0, 0, nullptr, nullptr, TData(1e-10));
    if (!optional_center.has_value()) {
        return std::nullopt;
    }
    auto center = optional_center.value() + x(0);
    return BoundingSphere<TData, 3>::from_center_and_iterator(center, x.flat_begin(), x.flat_end());
}

template <class TData, size_t tndim, class TRng>
std::optional<BoundingSphere<TData, tndim>> circumscribed_sphere(
    const std::vector<const FixedArray<TData, tndim>*>& R,
    TRng& rng)
{
    if (R.size() == 0) {
        THROW_OR_ABORT("Cannot compute bounding sphere for empty list");
    }
    if (R.size() > tndim + 1) {
        THROW_OR_ABORT("Too many points to compute trivial circumscribed sphere");
    }
    if (R.size() == 1) {
        return BoundingSphere<TData, tndim>(*R[0], (TData)0);
    }
    if (R.size() == 2) {
        return BoundingSphere<TData, tndim>(FixedArray<FixedArray<TData, tndim>, 2>{*R[0], *R[1]});
    }
    if (R.size() == 3) {
        return circumscribed_sphere(FixedArray<FixedArray<TData, tndim>, 3>{*R[0], *R[1], *R[2]}, rng);
    }
    if constexpr (tndim == 3) {
        if (R.size() == 4) {
            return circumscribed_sphere(FixedArray<FixedArray<TData, 3>, 4>{*R[0], *R[1], *R[2], *R[3]}, rng);
        }
    }
    THROW_OR_ABORT("Cannot compute trivial bounding sphere for the specified dimension and number of points");
}

/** From: https://en.wikipedia.org/wiki/Smallest-circle_problem
 * input: Finite sets P and R of points in the plane |R| ≤ 3.
 * output: Minimal disk enclosing P with R on the boundary.
 */
template <class TData, size_t tndim, class TRng>
std::optional<BoundingSphere<TData, tndim>> welzl(
    std::vector<const FixedArray<TData, tndim>*>& P,
    std::vector<const FixedArray<TData, tndim>*>& R,
    TRng& rng)
{
    if (P.empty() || (R.size() == tndim + 1)) {
        return circumscribed_sphere(R, rng);
    }
    size_t p_i = std::uniform_int_distribution<size_t>(0, P.size() - 1)(rng);
    const auto* p_c = P[p_i];
    P[p_i] = P[P.size() - 1];
    P.resize(P.size() - 1);
    if (!P.empty() || !R.empty()) {
        auto D = welzl(P, R, rng);
        if (!D.has_value()) {
            return std::nullopt;
        }
        if (D.value().contains(*p_c)) {
            P.resize(P.size() + 1);
            P[P.size() - 1] = P[p_i];
            P[p_i] = p_c;
            return D;
        }
    }
    if (R.capacity() <= R.size()) {
        THROW_OR_ABORT("Capacity of R too small");
    }
    R.push_back(p_c);
    auto result = welzl(P, R, rng);
    P.resize(P.size() + 1);
    P[P.size() - 1] = P[p_i];
    P[p_i] = p_c;
    R.resize(R.size() - 1);
    return result;
}

template <class TData, size_t tndim, class TRng>
BoundingSphere<TData, tndim> welzl_from_vector(
    std::vector<const FixedArray<TData, tndim>*>& P,
    TRng& rng)
{
    std::vector<const FixedArray<TData, tndim>*> R;
    R.reserve(P.size());
    for (size_t i = 0; i < 100; ++i) {
        auto result = welzl(P, R, rng);
        if (result.has_value()) {
            return result.value();
        }
        std::shuffle(P.begin(), P.end(), rng);
    }
    std::stringstream sstr;
    sstr << "Welzl did not succeed. Vertices:\n";
    for (const auto& p : P) {
        sstr << *p << '\n';
    }
    THROW_OR_ABORT(sstr.str());
}

template <class TData, size_t tndim, size_t tnpoints, class TRng>
BoundingSphere<TData, tndim> welzl_from_fixed(
    const FixedArray<FixedArray<TData, tndim>, tnpoints>& P,
    TRng& rng)
{
    std::vector<const FixedArray<TData, tndim>*> Pvec(tnpoints);
    for (size_t i = 0; i < tnpoints; ++i) {
        Pvec[i] = &P(i);
    }
    return welzl_from_vector(Pvec, rng);
}

template <class TData, size_t tndim, class TIterable, class TRng>
BoundingSphere<TData, tndim> welzl_from_iterator(
    const TIterable& P_begin,
    const TIterable& P_end,
    TRng& rng)
{
    std::vector<const FixedArray<TData, tndim>*> Pvec;
    Pvec.reserve(size_t(P_end - P_begin));
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
