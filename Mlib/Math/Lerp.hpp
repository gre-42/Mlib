#pragma once
#include <Mlib/Math/Funpack.hpp>

namespace Mlib {

template <class TData, class TAlpha>
auto lerp(const TData& a, const TData& b, const TAlpha& alpha) {
	return (TData)(funpack(a) + alpha * funpack(b - a));
}

}
