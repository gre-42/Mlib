#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

template <class TData>
Array<TData> logspace(const TData& from, const TData& to, size_t count) {
    return pow(TData{10}, linspace(from, to, count));
}

}
