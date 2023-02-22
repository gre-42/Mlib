#pragma once
#include <string>

namespace Mlib {

template <class TData>
class Array;

template <class TData>
Array<TData> load_heightmap_from_file(const std::string& filename);

}
