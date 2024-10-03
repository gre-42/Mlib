#pragma once
#include "Quad_3D.hpp"
#include "Triangle_3D.hpp"

using namespace Mlib;

template <class TPos, size_t tnvertices>
struct Polygon3D_t;

template <class TPos>
struct Polygon3D_t<TPos, 4> {
	using type = Quad3D<TPos>;
};

template <class TPos>
struct Polygon3D_t<TPos, 3> {
	using type = Triangle3D<TPos>;
};

template <class TPos, size_t tnvertices>
using Polygon3D = Polygon3D_t<TPos, tnvertices>::type;
