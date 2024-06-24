#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void test_fail(double zero = 0) {
    lerr() << 0. / zero;
}

void test_pass_inf(double zero = 0) {
    lerr() << 1. / zero;
}

void test_pass_std() {
    lerr() << 1. / 1.;
}

int main(int argc, char** argv) {
    test_fail();
    test_pass_inf();
    test_pass_std();
    enable_floating_point_exceptions();
    {
        TemporarilyIgnoreFloatingPointExeptions ignore_except;
        test_fail();
        test_pass_inf();
        test_pass_std();
    }
    // test_fail();
    return 0;
}
