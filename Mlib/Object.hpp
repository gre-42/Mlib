#pragma once

namespace Mlib {

class Object {
    Object(const Object&) = delete;
    Object& operator = (const Object&) = delete;
public:
    Object() = default;
    virtual ~Object() = default;
};

}
