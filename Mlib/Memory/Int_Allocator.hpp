#pragma once
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <concepts>
#include <iosfwd>
#include <set>
#include <stdexcept>
#include <unordered_set>

namespace Mlib {

template <std::integral TLarge, std::integral TSmall>
struct PeriodicIndex {
    explicit PeriodicIndex(TLarge large, TLarge period)
        : large{large}
        , modulus{integral_cast<TSmall>(large % period)}
    {}
    ~PeriodicIndex() = default;
    bool operator < (const PeriodicIndex& rhs) const {
        return modulus < rhs.modulus;
    }
    bool operator > (const PeriodicIndex& rhs) const {
        return modulus > rhs.modulus;
    }
    bool operator == (const PeriodicIndex& rhs) const {
        return modulus == rhs.modulus;
    }
    TLarge large;
    TSmall modulus;
};

template <std::integral TLarge, std::integral TSmall>
std::ostream& operator << (std::ostream& ostr, const PeriodicIndex<TLarge, TSmall>& index) {
    ostr << '(' << index.large << ", " << (TLarge)index.modulus << ')';
    return ostr;
}

template <std::integral TLarge, std::integral TSmall>
class IntAllocator {
    IntAllocator(const IntAllocator&) = delete;
    IntAllocator& operator = (const IntAllocator&) = delete;
    using Index = PeriodicIndex<TLarge, TSmall>;
public:
    explicit IntAllocator(TLarge capacity)
        : capacity_{capacity}
        , largest_{(TLarge)-1}
    {
        if (capacity > 10'000) {
            throw std::runtime_error("IntAllocator capacity too large");
        }
        for (size_t i = 0; i < capacity; ++i) {
            if (!free_indices_.emplace(i).second) {
                throw std::runtime_error("Could not insert free index");
            }
        }
    }
    ~IntAllocator() = default;
    Index allocate() {
        if (free_indices_.empty()) {
            throw std::runtime_error("Could not find a free index");
        }
        auto lower = integral_cast<TLarge>((TLarge(largest_ + 1) / capacity_) * capacity_);
        auto node = free_indices_.extract(free_indices_.begin());
        if (node.empty()) {
            verbose_abort("Could not extract free index");
        }
        auto large = integral_cast<TLarge>(lower + node.value());
        auto res = used_indices_.emplace(large, capacity_);
        if (!res.second) {
            verbose_abort("Could not insert free index " + std::to_string(large));
        }
        largest_ = std::max(lower, large);
        return *res.first;
    }
    void free(TLarge large) {
        auto node = used_indices_.extract(Index{large, capacity_});
        if (node.empty()) {
            verbose_abort("Could not erase index " + std::to_string(large));
        }
        if (!free_indices_.emplace(node.value().modulus).second) {
            verbose_abort("Could not occupy index " + std::to_string(large));
        }
    }
private:
    TLarge capacity_;
    TLarge largest_;
    std::unordered_set<TSmall> free_indices_;
    std::set<Index> used_indices_;
};

}
