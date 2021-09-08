#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

template <class TData>
Array<TData> log2space(const TData& from, const TData& to, size_t count) {
    return pow(TData{2}, linspace(from, to, count));
}

}
