#pragma once
#include <cstddef>

namespace Mlib {

template <class T>
struct alignas(T) ObjectBlob {
    std::byte data[sizeof(T)];
};

}
