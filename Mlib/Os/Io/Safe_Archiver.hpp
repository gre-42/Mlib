#pragma once
#include <cstdint>
#include <type_traits>

namespace Mlib {

template <class Archive>
class SafeArchiver {
public:
    inline SafeArchiver(Archive& archive)
        : archive_{archive}
    {}
    struct is_saving {
        static const bool value = Archive::is_saving;
    };
    template <class TElement>
    void operator () (TElement& element) {
        static_assert(!std::is_same_v<TElement, int64_t>);
        static_assert(!std::is_same_v<TElement, uint64_t>);
        archive_(element);
    }

private:
    Archive& archive_;
};

}
