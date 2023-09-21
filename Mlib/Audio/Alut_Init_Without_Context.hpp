#pragma once

namespace Mlib {

class AlutInitWithoutContext {
    AlutInitWithoutContext(const AlutInitWithoutContext &) = delete;
    AlutInitWithoutContext &operator=(const AlutInitWithoutContext &) = delete;

public:
    AlutInitWithoutContext();
    ~AlutInitWithoutContext();
};
    
}
