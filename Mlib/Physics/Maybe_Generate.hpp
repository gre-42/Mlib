#pragma once
#include <cstdint>

namespace Mlib {

class MaybeGenerate {
public:
    explicit MaybeGenerate();
    ~MaybeGenerate();
    void advance_time(float dt);
    uint32_t operator()(float generation_dt);
private:
    float lifetime_;
};

}
