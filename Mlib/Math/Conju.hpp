#pragma once
#include <Mlib/Scaled_Integer.hpp>
#include <complex>

namespace Mlib {

inline constexpr std::complex<float>  conju(const std::complex<float>& v) { return std::conj(v); }
inline constexpr std::complex<double> conju(const std::complex<double>& v) { return std::conj(v); }
inline constexpr std::complex<long double> conju(const std::complex<long double>& v) { return std::conj(v); }
inline constexpr float  conju(const float& v) { return v; }
inline constexpr double conju(const double& v) { return v; }
inline constexpr long double conju(const long double& v) { return v; }
template <class TInt, std::intmax_t denominator>
constexpr ScaledInteger<TInt, denominator> conju(const ScaledInteger<TInt, denominator>& v) {
    return v;
}

inline bool is_finite(unsigned char v) { return true; }
inline bool is_finite(const float& v) { return std::isfinite(v); }
inline bool is_finite(const double& v) { return std::isfinite(v); }
inline bool is_finite(const long double& v) { return std::isfinite(v); }
inline bool is_finite(const std::complex<float>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
inline bool is_finite(const std::complex<double>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
inline bool is_finite(const std::complex<long double>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
template <class TInt, std::intmax_t denominator>
constexpr bool is_finite(const ScaledInteger<TInt, denominator>& v) {
    return true;
}

}
