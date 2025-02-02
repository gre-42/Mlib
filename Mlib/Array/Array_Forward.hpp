#pragma once
#include <cstddef>

namespace Mlib {

template <class TData>
class Array;

class ArrayResizer;

class ArrayShape;

template <class TData>
class ArrayAxisView;

template <class TData>
class Vector;

template <class TData>
class SparseArrayCcs;

template <typename TData, size_t... tshape>
class FixedArray;

}
