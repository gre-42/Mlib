#pragma once

namespace Mlib {

class MaybeGenerate {
public:
    explicit MaybeGenerate();
    ~MaybeGenerate();
    void advance_time(float dt);
    bool operator()(float generation_dt);
private:
    float lifetime_;
};

}
