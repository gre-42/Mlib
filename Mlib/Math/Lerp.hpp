#pragma once

namespace Mlib {

template <class A, class B, class T>
auto lerp(const A& a, const B& b, const T& alpha) {
	return a + alpha * (b - a);
}

}
