#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Aim.hpp>
#include <Mlib/Physics/Objects/Gravity_Efp.hpp>
#include <Mlib/Physics/Objects/Rigid_Body.hpp>
#include <Mlib/Physics/Objects/Rigid_Primitives.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Physics/Power_To_Force.hpp>
#include <fenv.h>

using namespace Mlib;

void test_aim() {
    {
        FixedArray<float, 3> gun_pos{1, 2, 3};
        FixedArray<float, 3> target_pos{4, 2, 2};
        {
            float velocity = 20;
            float gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.0387768f);
            assert_isclose(aim.aim_offset, 0.122684f);
        }
        {
            float velocity = 10;
            float gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.157546f);
            assert_isclose(aim.aim_offset, 0.502367f);
        }
    }
    {
        {
            FixedArray<float, 3> gun_pos{1, 2, 3};
            FixedArray<float, 3> target_pos{4, 2, 3};
            float velocity = 10;
            float gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.149205f);
            assert_isclose(aim.aim_offset, 0.450965f);
        }
        {
            FixedArray<float, 3> gun_pos{1, 2, 3};
            FixedArray<float, 3> target_pos{5, 2, 3};
            float velocity = 10;
            float gravity = 9.8;
            Aim aim{gun_pos, target_pos, 1, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.111633f);
            assert_isclose(aim.aim_offset, 0.448397f);
        }
    }
}

void test_power_to_force_negative() {
    FixedArray<float, 3> n3{1, 0, 0};
    float P = 51484.9; // Watt, 70 PS
    FixedArray<float, 3> v3{0, 0, 0};
    float dt = 0.1;
    float m = 1000;
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(10, 20, 1e-1, 5e4, 5e4, INFINITY, n3, P, m, v3, dt, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), 32.0889, 1e-4);
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(10, 20, 1e-1, 5e4, 5e4, INFINITY, n3, -P, m, v3, dt, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), -26.4613, 1e-4);
}

void test_power_to_force_stiction() {
    FixedArray<float, 3> n3{1, 0, 0};
    float P = INFINITY;
    float g = 9.8;
    FixedArray<float, 3> v3{0, 0, 0};
    float dt = 0.016667;
    float m = 1000;
    float stiction_coefficient = 1;
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(10, 20, 1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, 4321, v3, dt, true);
        F += power_to_force_infinite_mass(10, 20, 1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, 4321, v3, dt, true);
        v3 += F / m * dt;
    }
    assert_isclose<float>(v3(0), 97.0226, 1e-4);
}

void test_power_to_force_P() {
    FixedArray<float, 3> n3{1, 0, 0};
    float P = 51484.9; // Watt, 70 PS
    FixedArray<float, 3> v3{0, 0, 0};
    float dt = 0.016667;
    float m = 1000;
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(10, 20, 1e-1, INFINITY, 1e3, INFINITY, n3, P, m / 20, v3, dt, true);
        F += power_to_force_infinite_mass(10, 20, 1e-1, INFINITY, 1e3, INFINITY, n3, P, m / 20, v3, dt, true);
        v3 += F / m * dt;
    }
    assert_isclose<float>(v3(0), 44.819, 1e-4);
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_aim();
    test_power_to_force_negative();
    test_power_to_force_stiction();
    test_power_to_force_P();
    return 0;
}
