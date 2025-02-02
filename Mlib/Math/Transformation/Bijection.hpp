#pragma once

namespace Mlib {

template <class T>
struct Bijection {
	T model;
	T view;
};

template <class T>
Bijection<T> operator * (const Bijection<T>& a, const Bijection<T>& b) {
	return {
        .model = a.model * b.model,
        .view = b.view * a.view
    };
}

}
