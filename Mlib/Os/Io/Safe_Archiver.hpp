#pragma once
#include <cstdint>
#include <type_traits>

namespace Mlib {

template <typename T>
concept NotInt64 = !std::is_same_v<T, int64_t> && !std::is_same_v<T, uint64_t>;

template <class Archive>
class SafeArchiver {
public:
    inline SafeArchiver(Archive& archive)
        : archive_{archive}
    {}
    struct is_saving {
        static const bool value = Archive::is_saving;
    };
    template <NotInt64 TElement>
    void operator () (TElement& element) {
        archive_(element);
    }
    template <NotInt64 TElement>
    void operator () (const TElement& element) {
        archive_(element);
    }
private:
    Archive& archive_;
};

}
