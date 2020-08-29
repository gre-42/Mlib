#pragma once
#include <complex>

template <class TData> inline TData conju(const TData& v) { return std::conj(v); }
template <> inline float conju<float>(const float& v) { return v; }
template <> inline double conju<double>(const double& v) { return v; }

template <class TData> inline bool is_finite(const TData& v) { return std::isfinite(v); }
template <> inline bool is_finite<std::complex<float>>(const std::complex<float>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
template <> inline bool is_finite<std::complex<double>>(const std::complex<double>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
