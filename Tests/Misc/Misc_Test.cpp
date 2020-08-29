#include <Mlib/Math/Math.hpp>
#include <Mlib/Resource_Ptr.hpp>
#include <fenv.h>
#include <iostream>

using namespace Mlib;

struct A { int a = 3; };

void test_resource_ptr() {
    resource_ptr_target<float> v{new float(5)};
    {
        resource_ptr<float> pv = v.instantiate();
        assert_isequal(*pv, 5.f);
    }
    resource_ptr_target<A> a{new A};
    {
        resource_ptr<A> b = a.instantiate();
        assert_isequal(b->a, 3);
    }
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_resource_ptr();
    return 0;
}
