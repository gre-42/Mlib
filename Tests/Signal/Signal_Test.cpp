#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Images/Svg.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>

using namespace Mlib;

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
        THROW_OR_ABORT("Could not open file \"" + filename + "\" for write");
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
        THROW_OR_ABORT("Could not write to file \"/" + filename + "\"");
    }
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
        test_pid();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
