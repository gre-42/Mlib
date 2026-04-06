#pragma once
#include <iosfwd>
#include <optional>

namespace Mlib {

template <class T>
class DefaultOptional {
public:
    static DefaultOptional<T> from_default(T default_value) {
        return DefaultOptional<T>{ default_value };
    }
    DefaultOptional& operator = (const T& other) {
        optional_ = other;
        return *this;
    }
    auto value_or_default() const {
        return optional_.value_or(default_value_);
    }
    auto value() const {
        return optional_.value();
    }
    bool has_value() const {
        return optional_.has_value();
    }
private:
    template <typename... Args>
    explicit DefaultOptional(T default_value)
    : default_value_{ std::move(default_value) }
    {}
    std::optional<T> optional_;
    T default_value_;
};

template <class T>
std::ostream& operator << (std::ostream& ostr, const DefaultOptional<T>& v) {
    return (ostr << v.value_or_default());
}

}
