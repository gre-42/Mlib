#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Optical_Flow.hpp>

using namespace Mlib;

void test_optical_flow() {
    ArrayShape shape{100, 12};
    size_t dt = 1;
    size_t width = 10;
    size_t window = 7;
    Array<float> image0 = random_array2<float>(shape, 1).row_range(0 * dt, 0 * dt + width);
    Array<float> image1 = random_array2<float>(shape, 1).row_range(1 * dt, 1 * dt + width);
    Array<float> image2 = random_array2<float>(shape, 1).row_range(2 * dt, 2 * dt + width);
    Array<float> flow;
    Array<bool> mask;
    optical_flow(image0, image1, &image2, ArrayShape{window, window}, 100.f, flow, mask);
    // lerr() << flow;
    assert_isclose(flow[1][4](5), -1.f);
}

int main(int argc, char** argv) {
    test_optical_flow();
    return 0;
}
