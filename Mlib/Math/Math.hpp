#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Array/Consteval_Workaround.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Math/Abs.hpp>
#include <Mlib/Math/Float_Type.hpp>
#include <Mlib/Math/Funpack.hpp>
#include <Mlib/Rvalue_Address.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <Mlib/Type_Traits/Get_Scalar.hpp>
#include <Mlib/Type_Traits/Scalar.hpp>
#include <climits>
#include <concepts>
#include <optional>
#include <random>
#include <sstream>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class PowerIterationDidNotConvergeError: public std::runtime_error {
public:
    PowerIterationDidNotConvergeError(const std::string& what):
        std::runtime_error(what)
    {}
};

inline bool scalar_isnan(float s) {
    return std::isnan(s);
}

inline bool scalar_isnan(double s) {
    return std::isnan(s);
}

inline bool scalar_isnan(int32_t s) {
    return false;
}

inline bool scalar_isnan(uint32_t s) {
    return false;
}

inline bool scalar_isnan(int64_t s) {
    return false;
}

inline bool scalar_isnan(uint64_t s) {
    return false;
}

inline bool scalar_isnan(bool s) {
    return false;
}

template <class TData> void identity_array(Array<TData>& a);
template <class TData> Array<TData> identity_array(size_t n);

template <class TData> void operator , (const Array<TData>& a, const Array<TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator + (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator - (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator * (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator / (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedA, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator < (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator > (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator <= (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator >= (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator == (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TDerivedB, class TData> auto operator != (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TData> auto operator + (const BaseDenseArray<TDerivedA, TData>& a, const TData& b);
template <class TDerivedB, class TData> auto operator + (const TData& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerivedA, class TData> auto operator - (const BaseDenseArray<TDerivedA, TData>& a, const TData& b);
template <class TDerivedB, class TData> auto operator - (const TData& a, const BaseDenseArray<TDerivedB, TData>& b);
template <class TDerived, class TData> auto operator * (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator * (const TData& a, const BaseDenseArray<TDerived, TData>& b);
template <class TDerived, class TData> auto operator / (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator / (const TData& a, const BaseDenseArray<TDerived, TData>& b);
template <class TDerived, class TData> auto operator < (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator <= (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator > (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator >= (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator == (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator != (const BaseDenseArray<TDerived, TData>& a, const TData& b);
template <class TDerived, class TData> auto operator ! (const BaseDenseArray<TDerived, TData>& a);

inline Array<bool> operator < (const ArrayShape& a, const ArrayShape& b);
inline Array<bool> operator <= (const ArrayShape& a, const ArrayShape& b);
inline Array<bool> operator == (const ArrayShape& a, size_t b);

template <class TDerived, class TFloat> auto norm(const BaseDenseArray<TDerived, std::complex<TFloat>>& a);
template <class TDerived, class TFloat> auto norm(const BaseDenseArray<TDerived, TFloat>& a);

template <class TDerived, class TData> auto isnan(const BaseDenseArray<TDerived, TData>& a);
template <class TDerived, class TData> auto isinf(const BaseDenseArray<TDerived, TData>& a);
template <class TDerived, class TData> auto isfinite(const BaseDenseArray<TDerived, TData>& a);

template <class TDerived, class TData> auto abs(const BaseDenseArray<TDerived, TData>& a);
template <class TDerived, class TFloat> auto abs(const BaseDenseArray<TDerived, std::complex<TFloat>>& a);

template <class TDerived> inline bool any(const BaseDenseArray<TDerived, bool>& a);
Array<bool> any(const Array<bool>& a, size_t axis);
template <class TDerived> inline bool all(const BaseDenseArray<TDerived, bool>& a);
Array<bool> all(const Array<bool>& a, size_t axis);

template <class TData> void randomize_array(Array<TData> a, size_t seed = 0);
template <class TData> Array<TData> random_array(const ArrayShape& shape, size_t seed = 0);
template <class TData> void randomize_array2(Array<TData> a, unsigned int seed = 0);
template <class TData> Array<TData> random_array2(const ArrayShape& shape, unsigned int seed);

template <class TData>
inline TData det2x2(const Array<TData>& a)
{
    assert(all(a.shape() == ArrayShape{2, 2}));
    return a(0, 0) * a(1, 1) - a(0, 1) * a(1, 0);
}

template <class TData>
inline TData det3x3(const Array<TData>& a)
{
    assert(all(a.shape() == ArrayShape{3, 3}));
    return
        a(0, 0) * (a(1, 1) * a(2, 2) - a(1, 2) * a(2, 1)) -
        a(0, 1) * (a(1, 0) * a(2, 2) - a(1, 2) * a(2, 0)) +
        a(0, 2) * (a(1, 0) * a(2, 1) - a(1, 1) * a(2, 0));
}

template <class TData>
inline TData trace2x2(const Array<TData>& a)
{
    assert(all(a.shape() == ArrayShape{2, 2}));
    return a(0, 0) + a(1, 1);
}

template <class TData>
inline Array<TData> det2x2_a(const Array<TData>& a)
{
    assert(a.shape(0) == 2);
    assert(a.shape(1) == 2);
    return a[0][0] * a[1][1] - a[0][1] * a[1][0];
}

template <class TData>
inline Array<TData> trace2x2_a(const Array<TData>& a)
{
    assert(a.shape(0) == 2);
    assert(a.shape(1) == 2);
    return a[0][0] + a[1][1];
}

template <class TData>
Array<TData> reconstruct_svd(
    const Array<TData>& u,
    const Array<typename FloatType<TData>::value_type>& s,
    const Array<TData>& vT)
{
    assert(u.ndim() == 2);
    assert(s.ndim() == 1);
    assert(vT.ndim() == 2);
    assert(u.shape(1) >= s.length()); // ECON: equal
    assert(vT.shape(0) == s.length());
    Array<TData> us(ArrayShape{u.shape(0), s.length()});
    for (size_t r = 0; r < u.shape(0); ++r) {
        for (size_t c = 0; c < s.length(); ++c) {
            us(r, c) = u(r, c) * s(c);
        }
    }
    return dot(us, vT);
}

template <class TData>
Array<TData> pinv_svd(
    const Array<TData>& u,
    const Array<typename FloatType<TData>::value_type>& s,
    const Array<TData>& vT)
{
    using float_type = typename FloatType<TData>::value_type;
    return reconstruct_svd(vT.H(), float_type(1) / s, u.H());
}

/*
 * http://rosettacode.org/wiki/Cholesky_decomposition#Python
 */
template <class TData>
std::optional<Array<TData>> cholesky(
    const Array<TData>& A,
    const typename FloatType<TData>::value_type& diag2_min = 0)
{
    assert(A.shape(0) == A.shape(1));
    Array<TData> L;
    L.resize(A.shape());
    for (size_t i = 0; i < A.shape(0); ++i) {
        for (size_t j = 0; j <= i; ++j) {
            TData s = 0;
            for (size_t k = 0; k < j; ++k) {
                s += L(i, k) * conju(L(j, k));
            }
            if (i == j) {
                auto diag2 = std::real(A(i, i)) - std::real(s);
                if ((diag2_min != 0) && (diag2 < diag2_min)) {
                    return std::nullopt;
                }
                L(i, j) = std::sqrt(diag2);
            } else {
                L(i, j) = (A(i, j) - s) / L(j, j);
            }
        }
    }
    return L;
}

/*
 * http://en.wikipedia.org/wiki/Triangular_matrix
 * backward substitution
 */
template <class TDerivedL, class TDerivedU, class TDerivedB, class TData>
Array<TData> solve_LU(
    const BaseDenseArray<TDerivedL, TData>& L,
    const BaseDenseArray<TDerivedU, TData>& U,
    const BaseDenseArray<TDerivedB, TData>& B)
{
    // Ax = b -> LUx = b. Then y is defined to be Ux
    assert(L->ndim() == 2);
    assert(U->ndim() == 2);
    assert(B->ndim() == 2);
    assert(L->shape(0) == L->shape(1)); // square
    assert(all(L->shape() == U->shape()));
    assert(B->shape(0) == U->shape(1));
    Array<TData> x, y;
    x.resize(B->shape());
    y.resize(B->shape());
    for (size_t v = 0; v < B->shape(1); ++v) {
        // Forward solve Ly = b
        for (size_t i = 0; i < B->shape(0); ++i) {
            y(i, v) = (*B)(i, v);
            for (size_t j = 0; j < i; ++j) {
                y(i, v) -= (*L)(i, j) * y(j, v);
            }
            y(i, v) /= (*L)(i, i);
        }
        // Backward solve Ux = y
        for (size_t i = B->shape(0) - 1; i != SIZE_MAX; --i) {
            x(i, v) = y(i, v);
            for (size_t j = i + 1; j < B->shape(0); ++j) {
                x(i, v) -= (*U)(i, j) * x(j, v);
            }
            x(i, v) /= (*U)(i, i);
        }
    }
    return x;
}

/*
 * Solves the equation A⁻¹B for a symmetric matrix A using Cholesky
 * decomposition.
 * [C    ] * x = [D     ]
 * [eps*I]       [eps*x0]
 *
 * alpha := eps^2
 * (C'C + alpha) * x = C'D + alpha * x0
 */
template <class TDerivedA, class TDerivedB, class TData>
std::optional<Array<TData>> solve_symm_inplace(
    BaseDenseArray<TDerivedA, TData>& A,
    BaseDenseArray<TDerivedB, TData>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const Array<TData>* x0 = nullptr,
    const typename FloatType<TData>::value_type& diag2_min = 0)
{
    assert(A->ndim() == 2);
    assert(A->shape(0) == A->shape(1));
    assert(B->ndim() == 2);
    assert(B->shape(0) == A->shape(1));
    if (x0 != nullptr) {
        assert(x0->ndim() == 2);
        assert(all(x0->shape() == B->shape()));
    }
    if (alpha != TData(0) ||
        beta != TData(0))
    {
        // OpenCV Levenberg-Marquardt
        for (size_t r = 0; r < A->template static_shape<0>(); ++r) {
            TData dr = alpha + beta * (*A)(r, r);
            (*A)(r, r) += dr;
            if (x0 != nullptr) {
                for (size_t c = 0; c < B->template static_shape<1>(); ++c) {
                    (*B)(r, c) += (*x0)(r, c) * dr;
                }
            }
        }
    }
    auto L = cholesky(*A, diag2_min);
    if (!L.has_value()) {
        return std::nullopt;
    }
    return solve_LU(*L, L->vH(), B);
}

template <class TDerivedB, class TData>
std::optional<Array<TData>> solve_symm(
    const Array<TData>& A,
    const BaseDenseArray<TDerivedB, TData>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const Array<TData>* x0 = nullptr,
    const TData& diag2_min = TData(0))
{
    Array<TData> AI(A);
    TDerivedB BI(*B);
    if (alpha != TData(0) ||
        beta != TData(0))
    {
        AI.reassign(A);
        if (x0 != nullptr) {
            BI.reassign(*B);
        }
    }
    return solve_symm_inplace(AI, BI, alpha, beta, x0, diag2_min);
}

template <class TDerivedB, class TData>
std::optional<Array<TData>> solve_symm_1d(
    const Array<TData>& A,
    const BaseDenseArray<TDerivedB, TData>& B,
    const TData& alpha = 0,
    const TData& beta = 0,
    const Array<TData>* x0 = nullptr,
    const typename FloatType<TData>::value_type& diag2_min = TData(0))
{
    assert(B->ndim() == 1);
    if (x0 != nullptr) {
        assert(x0->length() == A.shape(0));
    }
    auto res = solve_symm(
        A,
        B->as_column_vector(),
        alpha,
        beta,
        x0 != nullptr
            ? rvalue_address(x0->as_column_vector())
            : nullptr,
        diag2_min);
    if (!res.has_value()) {
        return std::nullopt;
    }
    return res->flattened();
}

template <class TArrayA, class TArrayB>
std::optional<Array<typename TArrayA::value_type>> lstsq_chol(
    const TArrayA& A,
    const TArrayB& B,
    const typename TArrayA::value_type& alpha = 0,
    const typename TArrayA::value_type& beta = 0,
    const decltype(dot2d(A.vH(), A))* dAT_A = nullptr,
    const Array<typename TArrayA::value_type>* dAT_B = nullptr,
    const typename FloatType<typename TArrayA::value_type>::value_type& diag2_min = 0)
{
    auto AT_A = dot2d(A.vH(), A);
    Array<typename TArrayA::value_type> AT_B = dot2d(A.vH(), B);
    if (dAT_A != nullptr) {
        assert(all(dAT_A->shape() == AT_A.shape()));
        AT_A += *dAT_A;
    }
    if (dAT_B != nullptr) {
        assert(all(dAT_B->shape() == AT_B.shape()));
        AT_B += *dAT_B;
    }
    return solve_symm_inplace(AT_A, AT_B, alpha, beta, (const Array<typename TArrayA::value_type>*)nullptr, diag2_min);
}

template <class TArray>
std::optional<Array<typename TArray::value_type>> lstsq_chol_1d(
    const TArray& A,
    const Array<typename TArray::value_type>& B,
    const typename TArray::value_type& alpha = 0,
    const typename TArray::value_type& beta = 0,
    const decltype(dot2d(A.vH(), A))* dAT_A = nullptr,
    const Array<typename TArray::value_type>* dAT_b = nullptr,
    const typename FloatType<typename TArray::value_type>::value_type& diag2_min = 0)
{
    assert(B.ndim() == 1);
    Array<typename TArray::value_type> dAT_B1;
    if (dAT_b != nullptr) {
        assert(dAT_b->ndim() == 1);
        dAT_B1 = dAT_b->as_column_vector();
    }
    auto res = lstsq_chol(
        A,
        B.as_column_vector(),
        alpha,
        beta,
        dAT_A,
        dAT_b == nullptr ? nullptr : &dAT_B1,
        diag2_min);
    if (!res.has_value()) {
        return std::nullopt;
    }
    return res->flattened();
}

template <class TArray>
void lstsq_chol_1d(
    std::optional<Array<typename TArray::value_type>>& result,
    const TArray& A,
    const Array<typename TArray::value_type>& B,
    const typename TArray::value_type& alpha = 0,
    const typename TArray::value_type& beta = 0,
    const decltype(dot2d(A.vH(), A))* dAT_A = nullptr,
    const Array<typename TArray::value_type>* dAT_b = nullptr,
    const typename FloatType<typename TArray::value_type>::value_type& diag2_min = 0)
{
    result = lstsq_chol_1d(A, B, alpha, beta, dAT_A, dAT_b, diag2_min);
}

template <class TData>
std::optional<Array<TData>> inv(const Array<TData>& a, const TData& diag2_min = TData(0)) {
    assert(a.ndim() == 2);
    assert(a.shape(0) == a.shape(1));
    return lstsq_chol(a, identity_array<TData>(a.shape(0)), diag2_min);
}

/*
 * Uniformly random array
 */
template <class TData>
void randomize_array(Array<TData> a, size_t seed) {
    Array<TData> fa = a.flattened();
    for (size_t i = 0; i < fa.length(); i++) {
        //a(c) = std::uniform_real_distribution<TData>()();
        using float_type = typename FloatType<TData>::value_type;
        fa(i) = (float_type)seed;
        seed = (seed * 27 + 47) % 1000;
    }
}

template <class TData>
Array<TData> random_array(const ArrayShape& shape, size_t seed) {
    Array<TData> a(shape);
    randomize_array(a, seed);
    return a;
}

/*
 * Uniformly random array
 */
template <class TData>
void randomize_array2(Array<TData> a, unsigned int seed) {
    assert(seed != 0);
    std::srand(seed);
    Array<TData> fa = a.flattened();
    for (size_t i=0; i<fa.length(); i++) {
        fa(i) = (TData)(std::rand() / double(RAND_MAX));
    }
}

template <class TData>
Array<TData> random_array2(const ArrayShape& shape, unsigned int seed) {
    Array<TData> a(shape);
    randomize_array2(a, seed);
    return a;
}

/*
 * Uniformly random array
 */
template <class TData>
void randomize_array3(Array<TData> a, unsigned int seed) {
    assert(seed != 0);
    // std::default_random_engine e(seed);
    std::mt19937 e{ seed };
    Array<TData> fa = a.flattened();
    for (size_t i = 0; i < fa.length(); i++) {
        fa(i) = TData(e() - e.min()) / TData(e.max());
    }
}

template <class TData>
Array<TData> random_array3(const ArrayShape& shape, unsigned int seed) {
    Array<TData> a(shape);
    randomize_array3(a, seed);
    return a;
}

template <class TData>
Array<TData> full(const ArrayShape& shape, const TData& value) {
    Array<TData> a(shape);
    a = value;
    return a;
}

template <class TData>
Array<TData> zeros(const ArrayShape& shape) {
    return full<TData>(shape, 0);
}

template <class TData>
Array<TData> ones(const ArrayShape& shape) {
    return full<TData>(shape, 1);
}

template <class TData>
Array<TData> nans(const ArrayShape& shape) {
    return full<TData>(shape, NAN);
}

template <class TData>
void identity_array(Array<TData>& a) {
    if (a.ndim() == 2) {
        for (size_t r = 0; r < a.shape(0); ++r){
            for (size_t c = 0; c < a.shape(1); ++c) {
                a(r, c) = (r == c);
            }
        }
    } else {
        if (a.ndim() == 0) {
            a() = 1;
        } else {
            a.shape().foreach([&](const ArrayShape& index) {
                a(index) = all(index == index(0));
            });
        }
    }
}

template <class TData>
Array<TData> identity_array(size_t n) {
    Array<TData> result;
    result.do_resize(ArrayShape{n, n});
    identity_array(result);
    return result;
}

template <class TData>
Array<TData> dirac_array(const ArrayShape& shape, const ArrayShape& index) {
    Array<TData> result = zeros<TData>(shape);
    result(index) = 1;
    return result;
}

/*
 * Outer product of two matrices.
 * outer2d(a, b) = dot(a, b.H())
 */
template <class TDerivedA, class TDerivedB, class TDerivedR, class TData>
void outer2d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b,
    BaseDenseArray<TDerivedR, TData>& result)
{
    assert_true(CW::ndim(*a) == 2);
    assert_true(CW::ndim(*b) == 2);
    assert_true(CW::ndim(*result) == 2);

    size_t aR = CW::static_shape<0>(*a);
    size_t aC = CW::static_shape<1>(*a);

    size_t bR = CW::static_shape<0>(*b);
    size_t bC = CW::static_shape<1>(*b);

    size_t rR = CW::static_shape<0>(*result);
    size_t rC = CW::static_shape<1>(*result);

    assert_true(aC == bC);
    assert_true(rR == aR);
    assert_true(rC == bR);
    if (rR > INT_MAX) {
        THROW_OR_ABORT("Too many rows in matrix");
    }
    #pragma omp parallel for if (CW::nelements(*result) > 200 * 200)
    for (int r = 0; r < (int)rR; ++r) {
        for (size_t c = 0; c < rC; ++c) {
            TData v = 0;
            for (size_t i = 0; i < aC; ++i) {
                v += (*a)((size_t)r, i) * conju((*b)(c, i));
            }
            (*result)((size_t)r, c) = v;
        }
    }
}

/*
 * Outer product of two matrices.
 * outer2d(a, b) = dot(a, b.H())
 */
template <class TDerivedA, class TDerivedB, class TData>
Array<TData> outer2d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    assert_true(CW::ndim(*a) == 2);
    assert_true(CW::ndim(*b) == 2);

    size_t aR = CW::static_shape<0>(*a);
    size_t aC = CW::static_shape<1>(*a);

    size_t bR = CW::static_shape<0>(*b);
    size_t bC = CW::static_shape<1>(*b);

    assert_true(aC == bC);
    Array<TData> result{ ArrayShape{aR, bR} };
    outer2d(a, b, result);
    return result;
}

/*
 * Outer product of two matrices (sum over last axes).
 * ND-wrapper around outer2d.
 * 2D: (a, b) = dot(a, b.H())
 */
template <class TDerivedA, class TDerivedB, class TData>
Array<TData> outer(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (b_l0, b_l1, n)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    if ((a->ndim() != 2) || (b->ndim() != 2)) {
        auto a2 = a->rows_as_1D();
        auto b2 = b->rows_as_1D();
        Array<TData> r = outer2d(a2, b2);
        ArrayShape r_shape =
            a->array_shape()
            .erased_last()
            .concatenated(
                b->array_shape()
                .erased_last());
        r.do_reshape(r_shape);
        return r;
    } else {
        return outer2d(a, b);
    }
}

template <class TData>
void operator , (
    const Array<TData>& a,
    const Array<TData>& b)
{
    THROW_OR_ABORT("Use dot/outer instead");
}

template <class TDerivedA, class TDerivedB, class TDerivedR, class TData>
void dot2d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b,
    BaseDenseArray<TDerivedR, TData>& result)
{
    assert_true(CW::ndim(*a) == 2);
    assert_true(CW::ndim(*b) == 2);
    assert_true(CW::ndim(*result) == 2);

    size_t aR = CW::static_shape<0>(*a);
    size_t aC = CW::static_shape<1>(*a);

    size_t bR = CW::static_shape<0>(*b);
    size_t bC = CW::static_shape<1>(*b);

    size_t rR = CW::static_shape<0>(*result);
    size_t rC = CW::static_shape<1>(*result);

    assert_true(aC == bR);
    assert_true(rR == aR);
    assert_true(rC == bC);
    if (rR > INT_MAX) {
        THROW_OR_ABORT("Too many rows in matrix");
    }
    #pragma omp parallel for if (CW::nelements(*result) > 200 * 200)
    for (int r = 0; r < (int)rR; ++r) {
        for (size_t c = 0; c <rC; ++c) {
            TData v = 0;
            for (size_t i = 0; i < aC; ++i) {
                v += (*a)((size_t)r, i) * (*b)(i, c);
            }
            (*result)((size_t)r, c) = v;
        }
    }
}

template <class TDerivedA, class TDerivedB, class TData>
Array<TData> dot2d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    assert_true(CW::ndim(*a) == 2);
    assert_true(CW::ndim(*b) == 2);

    size_t aR = CW::static_shape<0>(*a);
    size_t aC = CW::static_shape<1>(*a);

    size_t bR = CW::static_shape<0>(*b);
    size_t bC = CW::static_shape<1>(*b);

    assert_true(aC == bR);

    Array<TData> result{ArrayShape{aR, bC}};
    dot2d(a, b, result);
    return result;
}

template <class TDerivedA, class TDerivedB, class TData>
Array<TData> dot1d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    assert_true(CW::ndim(*a) == 2);
    assert_true(CW::ndim(*b) == 1);

    size_t aR = CW::static_shape<0>(*a);
    size_t aC = CW::static_shape<1>(*a);

    size_t bR = CW::static_shape<0>(*b);

    assert_true(aC == bR);
    Array<TData> result{ ArrayShape{ aR, 1 } };
    dot2d(a, b->reshaped(ArrayShape{ b->length(), 1 }), result);
    return result.flattened();
}

template <class TDerivedA, class TDerivedB, class TData>
Array<TData> dot(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    // a.shape = (a_l0, a_l1, n)
    // b.shape = (n, b_l0, b_l1)
    // r.shape = (a_l0, a_l1, b_r0, b_r1)
    if ((CW::ndim(*a) != 2) || (CW::ndim(*b) != 2)) {
        decltype(auto) a2 = a->rows_as_1D();
        decltype(auto) b2 = b->columns_as_1D();
        Array<TData> r = dot2d(a2, b2);
        ArrayShape r_shape =
            a->array_shape()
            .erased_last()
            .concatenated(
                b->array_shape()
                .erased_first());
        r.do_reshape(r_shape);
        return r;
    } else {
        return dot2d(a, b);
    }
}

template <class TDerivedA, class TDerivedB, class TData>
TData dot0d(
    const BaseDenseArray<TDerivedA, TData>& a,
    const BaseDenseArray<TDerivedB, TData>& b)
{
    assert_true(CW::ndim(*a) == 1);
    assert_true(CW::ndim(*b) == 1);

    size_t aC = CW::static_shape<0>(*a);
    size_t bR = CW::static_shape<0>(*b);

    assert_true(aC == bR);
    return dot(a, b)();
}

/**
 * Computes dot(a, b) with "a" 2d, "b" at least 1d
 */
template <class TData>
Array<TData> batch_dot(const Array<TData>& a, const Array<TData>& b) {
    assert_true(a.ndim() == 2);
    assert_true(b.ndim() >= 1);
    assert_true(a.shape(1) == b.shape(0));
    auto b_shape_tail = b.shape().erased_first();
    Array<TData> result{ ArrayShape{a.shape(0)}.concatenated(b_shape_tail) };
    for (size_t r = 0; r < a.shape(0); ++r) {
        Array<TData> v = zeros<TData>(b_shape_tail);
        for (size_t i = 0; i < a.shape(1); ++i) {
            v += a(r, i) * b[i];
        }
        result[r] = v;
    }
    return result;
}

template <class TFloat>
Array<TFloat> real(const Array<std::complex<TFloat>>& a) {
    return a.template applied<TFloat>([](const std::complex<TFloat>& v){ return v.real(); });
}

template <class TFloat>
Array<TFloat> imag(const Array<std::complex<TFloat>>& a) {
    return a.template applied<TFloat>([](const std::complex<TFloat>& v){ return v.imag(); });
}

template <class TDerived, class TFloat>
auto norm(const BaseDenseArray<TDerived, std::complex<TFloat>>& a) {
    return a->template applied<TFloat>([](const std::complex<TFloat>& v){ return std::norm(v); });
}

template <class TDerived, class TFloat>
auto norm(const BaseDenseArray<TDerived, TFloat>& a) {
    return a->applied([](const TFloat&v){ return std::norm(v); });
}

template <class TDerived, class TFloat>
auto abs(const BaseDenseArray<TDerived, std::complex<TFloat>>& a) {
    return a->template applied<TFloat>([](const std::complex<TFloat>& v){ return std::abs(v); });
}

template <class TDerived, class TFloat>
auto log(const BaseDenseArray<TDerived, TFloat>& a) {
    return a->applied([](const TFloat& v){ return std::log(v); });
}

template <class TDerived, class TFloat>
auto round(const BaseDenseArray<TDerived, TFloat>& a) {
    return a->applied([](const TFloat& v){ return std::round(v); });
}

/**
 * Computes x ** y
 */
template <class TDerived, class TFloat>
auto pow(const TFloat& x, const BaseDenseArray<TDerived, TFloat>& y) {
    return y->applied([&](const TFloat& v){ return std::pow(x, v); });
}

/**
 * Computes x ** y
 */
template <class TDerived, class TFloat>
auto pow(const BaseDenseArray<TDerived, TFloat>& x, const TFloat& y) {
    return x->applied([&](const TFloat& v){ return std::pow(v, y); });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator + (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return x + y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator - (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return x - y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator * (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return x * y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator / (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->array_array_binop(*b, [](const TData& x, const TData& y) { return x / y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator < (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x < y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator > (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x > y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator <= (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x <= y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator >= (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x >= y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator == (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x == y; });
}

template <class TDerivedA, class TDerivedB, class TData>
auto operator != (const BaseDenseArray<TDerivedA, TData>& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return a->template array_array_binop<bool>(*b, [](const TData& x, const TData& y) { return x != y; });
}

template <class TData>
Array<TData> operator && (const Array<TData>& a, const Array<TData>& b) {
    return a.array_array_binop(b, [](const TData& x, const TData& y) { return x && y; });
}

template <class TData>
Array<TData> operator || (const Array<TData>& a, const Array<TData>& b) {
    return a.array_array_binop(b, [](const TData& x, const TData& y) { return x || y; });
}

template <class TDerivedA, class TData>
auto operator + (const BaseDenseArray<TDerivedA, TData>& a, const TData& b) {
    return a->applied([&](const TData& x){ return x + b; });
}

template <class TDerivedB, class TData>
auto operator + (const TData& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return b->applied([&](const TData& x){ return a + x; });
}

template <class TDerivedA, class TData>
auto operator - (const BaseDenseArray<TDerivedA, TData>& a, const TData& b) {
    return a->applied([&](const TData& x){ return x - b; });
}

template <class TDerivedB, class TData>
auto operator - (const TData& a, const BaseDenseArray<TDerivedB, TData>& b) {
    return b->applied([&](const TData& x){ return a - x; });
}

template <class TDerived, class TData>
auto operator * (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->applied([&](const TData& x){ return x * b; });
}

template <class TDerived, class TData, std::integral I>
auto operator * (const BaseDenseArray<TDerived, TData>& a, I b) {
    return a->applied([&](const TData& x){ return x * b; });
}

template <class TDerived, class TData, std::floating_point F>
auto operator * (const BaseDenseArray<TDerived, TData>& a, F b) {
    return a->applied([&](const TData& x){ return x * b; });
}

template <class TDerived, class TData>
auto operator * (const TData& a, const BaseDenseArray<TDerived, TData>& b) {
    return b * a;
}

template <class TDerived, class TData>
auto operator / (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->applied([&](const TData& x){ return x / b; });
}

template <class TDerived, class TData, std::integral I>
auto operator / (const BaseDenseArray<TDerived, TData>& a, I b) {
    return a->applied([&](const TData& x){ return x / b; });
}

template <class TDerived, class TData, std::floating_point F>
auto operator / (const BaseDenseArray<TDerived, TData>& a, F b) {
    return a->applied([&](const TData& x){ return x / b; });
}

template <class TDerived, class TData>
auto operator / (const TData& a, const BaseDenseArray<TDerived, TData>& b) {
    return b->applied([&](const TData& x){ return a / x; });
}

template <class TFloat>
Array<std::complex<TFloat>> operator / (const Array<std::complex<TFloat>>& a, const TFloat& b) {
    return a.applied([&](const std::complex<TFloat>& x){ return x / b; });
}

template <class TFloat>
Array<std::complex<TFloat>> operator / (const std::complex<TFloat>& a, const Array<std::complex<TFloat>>& b) {
    return b.applied([&](const std::complex<TFloat>& x){ return a / x; });
}

template <class TDerived, class TData>
auto abs(const BaseDenseArray<TDerived, TData>& a) {
    return a->applied([&](const TData& x){ return abs(x); });
}

template <class TDerived, class TData>
auto sqrt(const BaseDenseArray<TDerived, TData>& a) {
    return a->applied([&](const TData& x){ return std::sqrt(x); });
}

template <class TDerived, class TData>
auto exp(const BaseDenseArray<TDerived, TData>& a) {
    return a->applied([&](const TData& x){ return std::exp(x); });
}

template <Scalar T>
inline T sign(const T& a) {
    return (T)(a == 0 ? 0 : a < 0 ? -1 : 1);
}

template <class TDerived, class TData>
inline auto sign(const BaseDenseArray<TDerived, TData>& a) {
    return a->applied([&](const TData& x){ return sign(x); });
}

template <class TDerived, class TData>
auto operator < (const BaseDenseArray<TDerived, TData>& a, const TData& b){
    return a->template applied<bool>([&](const TData& x){ return x < b; });
}

template <class TDerived, class TData>
auto operator <= (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->template applied<bool>([&](const TData& x){ return x <= b; });
}

template <class TDerived, class TData>
auto operator > (const BaseDenseArray<TDerived, TData>& a, const TData& b){
    return a->template applied<bool>([&](const TData& x){ return x > b; });
}

template <class TDerived, class TData>
auto operator >= (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->template applied<bool>([&](const TData& x){ return x >= b; });
}

template <class TDerived, class TData>
auto operator == (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->template applied<bool>([&](const TData& x){ return x == b; });
}

template <class TDerived, class TData>
auto operator != (const BaseDenseArray<TDerived, TData>& a, const TData& b) {
    return a->template applied<bool>([&](const TData& x){ return x != b; });
}

template <class TDerived, class TData>
auto operator ! (const BaseDenseArray<TDerived, TData>& a) {
    return a->template applied<bool>([&](const TData& x){ return !x; });
}

template <class TCompare>
inline Array<bool> shape_shape_comparison(const ArrayShape& a, const ArrayShape& b, const TCompare &compare) {
    assert(a.ndim() == b.ndim());
    Array<bool> result;
    result.resize(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = compare(a(i), b(i));
    }
    return result;
}

template <class TCompare>
inline Array<bool> shape_comparison(const ArrayShape& a, const TCompare &compare) {
    Array<bool> result;
    result.resize(a.ndim());
    for (size_t i = 0; i < a.ndim(); i++) {
        result(i) = compare(a(i));
    }
    return result;
}

inline Array<bool> operator == (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x == y; });
}

inline Array<bool> operator != (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x != y; });
}

inline Array<bool> operator < (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x < y; });
}

inline Array<bool> operator <= (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x <= y; });
}

inline Array<bool> operator > (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x > y; });
}

inline Array<bool> operator >= (const ArrayShape& a, const ArrayShape& b) {
    return shape_shape_comparison(a, b, [](size_t x, size_t y){ return x >= y; });
}

inline Array<bool> operator == (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x == b; });
}

inline Array<bool> operator != (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x != b; });
}

inline Array<bool> operator < (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x < b; });
}

inline Array<bool> operator <= (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x <= b; });
}

inline Array<bool> operator > (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x > b; });
}

inline Array<bool> operator >= (const ArrayShape& a, size_t b) {
    return shape_comparison(a, [&](size_t x){ return x >= b; });
}

template <class TDerived>
inline bool any(const BaseDenseArray<TDerived, bool>& a) {
    for (bool value : a->flat_iterable()) {
        if (value) {
            return true;
        }
    }
    return false;
}

inline Array<bool> any(const Array<bool>& a, size_t axis) {
    return a.apply_over_axis(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<bool>& af, Array<bool>& rf)
        {
            rf(i, k) = false;
            for (size_t h = 0; h < a.shape(axis); ++h) {
                rf(i, k) |= af(i, h, k);
            }
        });
}

template <class TDerived>
inline bool all(const BaseDenseArray<TDerived, bool>& a) {
    for (bool value : a->flat_iterable()) {
        if (!value) {
            return false;
        }
    }
    return true;
}

inline Array<bool> all(const Array<bool>& a, size_t axis) {
    return a.apply_over_axis(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<bool>& af, Array<bool>& rf)
        {
            rf(i, k) = true;
            for (size_t h = 0; h < a.shape(axis); ++h) {
                rf(i, k) &= af(i, h, k);
            }
        });
}

template <class TDerived, class TData>
inline size_t count_nonzero(const BaseDenseArray<TDerived, TData>& a) {
    size_t result = 0;
    for (const TData& value : a->flat_iterable()) {
        if (value) {
            ++result;
        }
    }
    return result;
}

template <class TDerived, class TData>
TData sum(const BaseDenseArray<TDerived, TData>& a) {
    using ScalarType = scalar_type_t<TData>;
    TData result((ScalarType)0);
    for (const TData& v : a->flat_iterable()) {
        result += v;
    }
    return result;
}

template <class TData>
Array<TData> sum(const Array<TData>& x, size_t axis) {
    return x.apply_over_axis(axis, ApplyOverAxisType::REDUCE,
        [&](size_t i, size_t k, const Array<TData>& xf, Array<TData>& rf)
        {
            rf(i, k) = 0;
            for (size_t h = 0; h < x.shape(axis); ++h) {
                rf(i, k) += xf(i, h, k);
            }
        });
}

template <class TDerived, class TData>
TData prod(const BaseDenseArray<TDerived, TData>& a) {
    TData result(1);
    for (const TData& v : a->flat_iterable()) {
        result *= v;
    }
    return result;
}

template <class T>
constexpr inline auto squared(const T& a) {
    auto fa = funpack(a);
    return fa * fa;
}

template <class T>
constexpr inline auto cubed(const T& a) {
    auto fa = funpack(a);
    return fa * fa * fa;
}

template <class TData>
bool isclose(const TData& a, const TData& b, typename FloatType<TData>::value_type atol = 1e-6) {
    return
        (a == std::numeric_limits<TData>::infinity() &&
         b == std::numeric_limits<TData>::infinity()) ||
        (std::isnan(a) && std::isnan(b)) ||
        (!std::isnan(a) && !std::isnan(b) && (std::abs(a - b) < atol));
}

template <class TData>
bool isclose(const std::complex<TData>& a, const std::complex<TData>& b, typename FloatType<TData>::value_type atol = 1e-6) {
    return (
        isclose(std::real(a), std::real(b), atol) &&
        isclose(std::imag(a), std::imag(b), atol));

}

template <class TData>
bool isequal(const TData& a, const TData& b) {
    return
        (scalar_isnan(a) && scalar_isnan(b)) ||
        (!scalar_isnan(a) && !scalar_isnan(b) && (a == b));
}

template <class TData>
void assert_isclose(const TData& a, const TData& b, typename FloatType<TData>::value_type atol = (typename FloatType<TData>::value_type)1e-6) {
    if (!isclose(a, b, atol)) {
        std::stringstream sstr;
        sstr << "Numbers not close (atol=" << atol << "): " <<
            a << ", " << b;
        THROW_OR_ABORT(sstr.str());
    }
}

template <class TData>
void assert_allclose(const Array<TData>& a, const Array<TData>& b, typename FloatType<TData>::value_type atol = (typename FloatType<TData>::value_type)1e-6) {
    if ((a.ndim() != b.ndim()) || any(a.shape() != b.shape())) {
        std::stringstream sstr;
        sstr << "Shape mismatch: " << a.shape() << ", " << b.shape();
        THROW_OR_ABORT(sstr.str());
    }
    a.shape().foreach([&](const ArrayShape& index) {
        if (!isclose(a(index), b(index), atol)) {
            std::stringstream sstr;
            sstr << std::setprecision(18) << "Numbers not close (atol=" << atol << ") at " <<
                index << ": " << a(index) << ", " << b(index);
            THROW_OR_ABORT(sstr.str());
        }
    });
}

template <class TData>
void assert_allequal(const Array<TData>& a, const Array<TData>& b) {
    if ((a.ndim() != b.ndim()) || any(a.shape() != b.shape())) {
        std::stringstream sstr;
        sstr << "Shape mismatch: " << a.shape() << ", " << b.shape();
        THROW_OR_ABORT(sstr.str());
    }
    a.shape().foreach([&](const ArrayShape& index) {
        if (!isequal(a(index), b(index))) {
            std::stringstream sstr;
            sstr << "Numbers not identical at " <<
                index << ": " << a(index) << ", " << b(index);
            THROW_OR_ABORT(sstr.str());
        }
    });
}

template <class TData>
void assert_isequal(const TData& a, const TData& b) {
    if (!isequal(a, b)) {
        THROW_OR_ABORT(std::to_string(a) + " does not equal " + std::to_string(b));
    }
}

inline void assert_shape_equals(const ArrayShape& shape1, const ArrayShape& shape2) {
    assert_allclose<float>(
        Array<float>::from_shape(shape1),
        Array<float>::from_shape(shape2));
}

template <class TDerived, class TData>
auto isnan(const BaseDenseArray<TDerived, TData>& a) {
    return a->template applied<bool>([](const TData& v){ return std::isnan(v); });
}

template <class TDerived, class TData>
auto isinf(const BaseDenseArray<TDerived, TData>& a) {
    return a->template applied<bool>([](const TData& v){ return std::isinf(v); });
}

template <class TDerived, class TData>
auto isfinite(const BaseDenseArray<TDerived, TData>& a) {
    return a->template applied<bool>([](const TData& v){ return is_finite(v); });
}

template <class TData>
void nans_to_mask(
    const Array<TData>& image,
    Array<TData>& image0,
    Array<TData>& mask)
{
    image0.do_resize(image.shape());
    mask.do_resize(image.shape());
    Array<TData> image1D = image.flattened();
    Array<TData> mask1D = mask.flattened();
    Array<TData> image0_1D = image0.flattened();
    for (size_t i = 0; i < image1D.length(); ++i) {
        if (!std::isnan(image1D(i))) {
            mask1D(i) = 1;
            image0_1D(i) = image1D(i);
        } else {
            mask1D(i) = 0;
            image0_1D(i) = 0;
        }
    }
}

template <class TData>
Array<TData> substitute_nans(const Array<TData>& a, const TData& value) {
    return a.applied([&](const TData& x){ return std::isnan(x) ? value : x; });
}

template <class TData>
Array<TData> normalized_l2(const Array<TData>& a) {
    return a / TData(std::sqrt(sum(squared(a))));
}

}
