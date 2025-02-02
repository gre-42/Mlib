#pragma once
#include <memory>

namespace Mlib {

class ITrailExtender;

class ITrailStorage {
public:
    virtual ~ITrailStorage() = default;
    virtual std::unique_ptr<ITrailExtender> add_trail_extender() = 0;
};

}
