#pragma once
#include <complex>

inline std::complex<float>  conju(const std::complex<float>& v) { return std::conj(v); }
inline std::complex<double> conju(const std::complex<double>& v) { return std::conj(v); }
inline float  conju(const float& v) { return v; }
inline double conju(const double& v) { return v; }

inline bool is_finite(unsigned char v) { return true; }
inline bool is_finite(const float& v) { return std::isfinite(v); }
inline bool is_finite(const double& v) { return std::isfinite(v); }
inline bool is_finite(const std::complex<float>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
inline bool is_finite(const std::complex<double>& v) { return std::isfinite(v.real()) && std::isfinite(v.imag()) ; }
