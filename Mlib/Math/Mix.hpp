#pragma once

namespace Mlib {

template <class TData, class TAlpha>
TData mix(const TData& a, const TData& b, const TAlpha& alpha) {
	return a * ((TAlpha)1 - alpha) + alpha * b;
}

}
