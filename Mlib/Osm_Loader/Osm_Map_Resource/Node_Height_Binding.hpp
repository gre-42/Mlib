#pragma once
#include <Mlib/Strings/String.hpp>
#include <compare>
#include <stdexcept>
#include <string>

namespace Mlib {

class NodeHeightBinding {
public:
    NodeHeightBinding()
    : value_{"<undefined>"}
    {}
    NodeHeightBinding& operator = (const std::string& v) {
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
        return value_ == v;
    }
    const std::string& str() const {
        return value_;
    }
private:
    std::string value_;
};

}
