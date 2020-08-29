#pragma once

template <class TData> struct FloatType { typedef typename TData::value_type value_type; };
template <> struct FloatType<float> { typedef float value_type; };
template <> struct FloatType<double> { typedef double value_type; };
