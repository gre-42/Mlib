#pragma once
#include <fenv.h>

namespace Mlib {

class IgnoreExcept {
public:
    inline IgnoreExcept() {
        fpeflags_ = fegetexcept();
        fedisableexcept(FE_ALL_EXCEPT);
    }
    inline ~IgnoreExcept() {
        feenableexcept(fpeflags_);
    }
private:
    int fpeflags_;
};

}
