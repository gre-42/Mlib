#include <Mlib/Stats/Linspace.hpp>

namespace Mlib {

template <class TData>
Array<TData> logspace(const TData& from, const TData& to, size_t count, const TData& base = TData{10}) {
    return pow(base, linspace(from, to, count));
}

}
