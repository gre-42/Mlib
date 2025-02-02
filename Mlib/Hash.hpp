#pragma once
#include <cstddef>
#include <functional>

namespace Mlib {

// From: https://stackoverflow.com/questions/2590677/how-do-i-combine-hash-values-in-c0x
class Hasher {
public:
    explicit Hasher(std::size_t seed = 0xc0febabe)
        : seed{ seed }
    {}
    void combine() const {
        // Do nothing
    }
    template <typename T, typename... Rest>
    inline void combine(const T& v, const Rest&... rest) {
        std::hash<T> hasher;
        seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
        combine(rest...);
    }
    operator std::size_t() const {
        return seed;
    }
private:
    std::size_t seed;
};

template <typename... Args>
inline std::size_t hash_combine(const Args&... args) {
    Hasher hasher{ 0xc0febabe };
    hasher.combine(args...);
    return hasher;
}

}
