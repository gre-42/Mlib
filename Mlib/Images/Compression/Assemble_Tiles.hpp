#pragma once

namespace Mlib {

template <class TData>
class Array;
struct FragmentAssembly;

Array<float> assemble_tiles_compute_ols(FragmentAssembly& fa);
Array<float> assemble_tiles(const FragmentAssembly& fa);

}
