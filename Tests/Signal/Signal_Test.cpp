#include <Mlib/Floating_Point_Exceptions.hpp>
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
    size_t n = 150;
    float dt0 = 0.056f;
    float dt1 = (float)n * dt0;
    PidController<float, float> pid0{
        2.f,        // P
        0.5f,       // I
        1.7f,       // D
        0.2f };     // A
    auto pid1 = pid0.changed_time_step(dt0, dt1);
    Pms p0;
    Pms p1;
    float target = 5.f;
    for (size_t i = 0; i < 20; ++i) {
        p0.move(pid0(target - p0.x), dt0);
        for (size_t i = 0; i < n; ++i) {
            p1.move(pid1(target - p1.x), dt1);
        }
        linfo() << p0.x << " | " << p1.x << " | " << (p0.x - p1.x);
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
