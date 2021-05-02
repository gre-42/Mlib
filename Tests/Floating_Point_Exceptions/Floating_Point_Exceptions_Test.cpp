#include <Mlib/Floating_Point_Exceptions.hpp>
#include <iostream>

using namespace Mlib;

void test_fail(double zero = 0) {
    std::cerr << 0. / zero << std::endl;
}

void test_pass_inf(double zero = 0) {
    std::cerr << 1. / zero << std::endl;
}

void test_pass_std() {
    std::cerr << 1. / 1. << std::endl;
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
