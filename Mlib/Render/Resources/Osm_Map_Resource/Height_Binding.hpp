#pragma once
#include <compare>
#include <stdexcept>
#include <string>

namespace Mlib {

class HeightBinding {
public:
    HeightBinding()
    : value_{"<undefined>"}
    {}
    HeightBinding& operator = (const std::string& v) {
        if ((value_ != "<undefined>") && (v != value_)) {
            throw std::runtime_error("Height binding already set to a different value");
        }
        if (v == "<undefined>") {
            throw std::runtime_error("Height binding value forbidden");
        }
        value_ = v;
        return *this;
    }
    std::strong_ordering operator <=> (const std::string& v) const {
        if (value_ == "<undefined>") {
            throw std::runtime_error("Height binding undefiend");
        }
        return value_ <=> v;
    }
    std::strong_ordering operator <=> (const char* v) const {
        return *this <=> std::string(v);
    }
    bool operator == (const char* v) const {
        return (*this <=> v) == std::strong_ordering::equal;
    }
    const std::string& str() const {
        return value_;
    }
private:
    std::string value_;
};

}
