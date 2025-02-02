#pragma once
#include <Mlib/Throw_Or_Abort.hpp>
#include <compare>
#include <cstdint>
#include <optional>

namespace Mlib {

class CachedHash {
public:
    inline CachedHash& operator = (size_t value) {
        if (value_.has_value()) {
            THROW_OR_ABORT("Hash already computed");
        }
        value_ = value;
        return *this;
    }
    inline size_t operator * () const {
        return *value_;
    }
    inline size_t get() const {
        if (!value_.has_value()) {
            THROW_OR_ABORT("Hash not computed");
        }
        return *value_;
    }
    inline void reset() {
        value_.reset();
    }
    inline bool has_value() const {
        return value_.has_value();
    }
    bool operator < (const CachedHash&) const {
        return false;
    }
    bool operator == (const CachedHash&) const {
        return true;
    }
    bool operator != (const CachedHash&) const {
        return false;
    }
    inline bool operator > (const CachedHash&) const {
        return false;
    }
    inline std::strong_ordering operator <=> (const CachedHash&) const {
        return std::strong_ordering::equal;
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(value_);
    }
private:
    std::optional<size_t> value_;
};

}
