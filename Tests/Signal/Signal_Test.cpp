#include <Mlib/Images/Normalize_Integral.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <Mlib/Signal/Resample_1D.hpp>

using namespace Mlib;

void test_resample_1d() {
    linfo() << resample_1d(Array<float>{2.f, 4.f, 6.f}, 30.f, 15.f);
    linfo() << resample_1d(Array<float>{2.f, 4.f, 6.f, 8.f}, 30.f, 15.f);
    linfo() << resample_1d(Array<float>{2.f, 4.f, 6.f, 8.f}, 15.f, 30.f);
    linfo() << resample_1d(Array<float>{2.f}, 15.f, 30.f);
}

void test_normalize_integral() {
    linfo() << normalized_integral<float>(Array<int16_t>{1, 2, (int16_t)(0.5f * INT16_MAX), 4, INT16_MIN, INT16_MAX}, -1.f, 1.f);
    linfo() << normalized_integral<double>(Array<int16_t>{1, 2, (int16_t)(0.5 * INT16_MAX), 4, INT16_MIN, INT16_MAX}, -1.f, 1.f);
    linfo() << denormalized_integral<int16_t>(Array<float>{-0.9f, 0.f, 0.4f, 0.5f, 0.6f}, -1.f, 1.f);
    linfo() << denormalized_integral<int16_t>(Array<double>{-0.9, 0., 0.4, 0.5, 0.6}, -1., 1.);
}

struct Pms {
    float x = 2.f;
    float v = 0.f;
    void move(float a, float dt) {
        v += a * dt;
        x += v * dt;
    }
};

void test_pid() {
    std::string filename = "TestOut/pid.svg";
    auto f = create_ofstream(filename);
    if (f->fail()) {
        throw std::runtime_error("Could not open file \"" + filename + "\" for write");
    }
    Svg<float> svg{*f, 600, 500};
    size_t n = 150;
    float dt0 = 0.056f;
    float dt1 = dt0 / integral_to_float<float>(n);
    // To find the optimal parameters, set all of them except one to zero.
    PidController<float, float> pid0{
        2.f,        // P
        0.5f,       // I
        20.7f,      // D
        0.9f };     // A
    auto pid1 = pid0;
    Pms p0;
    Pms p1;
    float target = 5.f;
    std::vector<float> x0;
    std::vector<float> y0;
    std::vector<float> x1;
    std::vector<float> y1;
    auto steps = 200u;
    x0.reserve(steps);
    y0.reserve(steps);
    x1.reserve(steps);
    y1.reserve(steps);
    for (size_t i = 0; i < steps; ++i) {
        p0.move(pid0(target - p0.x), dt0);
        for (size_t i = 0; i < n; ++i) {
            p1.move(pid1(target - p1.x, dt0, dt1), dt1);
        }
        x0.push_back(integral_to_float<float>(i));
        y0.push_back(p0.x);
        x1.push_back(integral_to_float<float>(i));
        y1.push_back(p1.x);
        linfo() << p0.x << " | " << p1.x << " | " << (p0.x - p1.x);
    }
    svg.plot_multiple<float>({x0, x1}, {y0, y1});
    svg.finish();
    f->flush();
    if (f->fail()) {
        throw std::runtime_error("Could not write to file \"/" + filename + "\"");
    }
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
        test_resample_1d();
        test_normalize_integral();
        test_pid();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
