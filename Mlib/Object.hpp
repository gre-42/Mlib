#pragma once

namespace Mlib {

class Object {
    Object(const Object&) = delete;
    Object& operator = (const Object&) = delete;
    bool operator == (const Object&) const = delete;
public:
    Object() = default;
    virtual ~Object() = default;
};

}
