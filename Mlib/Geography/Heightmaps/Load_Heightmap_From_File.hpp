#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <string>

namespace Mlib {

template <class TData>
class Array;

template <class TData>
Array<TData> load_heightmap_from_file(const Utf8Path& filename);

}
