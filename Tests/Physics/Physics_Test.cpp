#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

void test_aim() {
    {
        FixedArray<double, 3> gun_pos{1., 2., 3.};
        FixedArray<double, 3> target_pos{4., 2., 2.};
        {
            double velocity = 20;
            double gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.0387768);
            assert_isclose(aim.aim_offset, 0.122684);
        }
        {
            double velocity = 10;
            double gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.157546);
            assert_isclose(aim.aim_offset, 0.502367);
        }
    }
    {
        {
            FixedArray<double, 3> gun_pos{1., 2., 3.};
            FixedArray<double, 3> target_pos{4., 2., 3.};
            double velocity = 10;
            double gravity = 9.8;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.149205);
            assert_isclose(aim.aim_offset, 0.450965);
        }
        {
            FixedArray<double, 3> gun_pos{1., 2., 3.};
            FixedArray<double, 3> target_pos{5., 2., 3.};
            double velocity = 10;
            double gravity = 9.8;
            Aim aim{gun_pos, target_pos, 1, velocity, gravity, 1e-7, 10};
            assert_isclose(aim.angle, 0.111633);
            assert_isclose(aim.aim_offset, 0.448397);
        }
    }
}

void test_power_to_force_negative() {
    FixedArray<float, 3> n3{1.f, 0.f, 0.f};
    float P = 51484.9f; // Watt, 70 hp
    FixedArray<float, 3> v3{0.f, 0.f, 0.f};
    float dt = 0.1f;
    float m = 1000;
    float alpha0 = 0.2f;
    for (float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass((float)1e4, (float)1e-1, (float)5e4, (float)5e4, INFINITY, n3, P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), 32.4703f, (float)1e-4);
    for (float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass((float)1e4, (float)1e-1, (float)5e4, (float)5e4, INFINITY, n3, -P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), -26.8054f, (float)1e-4);
}

void test_power_to_force_stiction_normal() {
    FixedArray<float, 3> n3{1.f, 0.f, 0.f};
    float P = INFINITY;
    float g = 9.8f;
    FixedArray<float, 3> v3{0.f, 0.f, 0.f};
    float dt = 0.016667f;
    float m = 1000;
    float stiction_coefficient = 1;
    float alpha0 = 0.2f;
    for (float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass((float)1e4, (float)1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        F += power_to_force_infinite_mass((float)1e4, (float)1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        v3 += F / m * dt;
    }
    assert_isclose<float>(v3(0), 97.0226f, (float)1e-4);
}
// Infinite max_stiction_force no longer supported
//
// void test_power_to_force_P_normal() {
//     FixedArray<float, 3> n3{1, 0, 0};
//     float P = 51484.9; // Watt, 70 hp
//     FixedArray<float, 3> v3{0, 0, 0};
//     float dt = 0.01667;
//     float m = 1000;
//     for (float t = 0; t < 10; t += dt) {
//         auto F = power_to_force_infinite_mass(10, 1e-1, INFINITY, 1e3, INFINITY, n3, P, m / 20, v3, dt, true);
//         F += power_to_force_infinite_mass(10, 1e-1, INFINITY, 1e3, INFINITY, n3, P, m / 20, v3, dt, true);
//         v3 += F / m * dt;
//     }
//     assert_isclose<float>(v3(0), 44.819, 1e-4);
// }

// void test_power_to_force_stiction_tangential() {
//     FixedArray<float, 3> n3{1, 0, 0};
//     float P = 0;
//     float g = 9.8;
//     float dt = 0.01667;
//     float m = 1000;
//     float stiction_coefficient = 1;
//     auto get_force = [&](float v){
//         FixedArray<float, 3> v3{0, v, 0};
//         return power_to_force_infinite_mass(10, 20, 1e-1, g * m * stiction_coefficient, 1e3, INFINITY, n3, P, 4321, v3, dt, true);
//     };
//     Array<float> vs = linspace(0.f, 5 / 3.6f, 100.f);
//     Array<float> fs{ArrayShape{0}};
//     for (float v : vs.flat_iterable()) {
//         fs.append(get_force(v)(1));
//     }
//     Array<float>({vs, fs}).T().save_txt_2d("vf.m");
//     // assert_isclose<float>(get_force(1.2)(1), 97.0226, 1e-4);
// }

void test_com() {
    PhysicsEngineConfig cfg;

    RigidBodies rbs{ cfg };
    float mass = 123.f * kg;
    FixedArray<float, 3> size{2 * meters, 3 * meters, 4 * meters};
    FixedArray<float, 3> com0{0 * meters, 0 * meters, 0 * meters};
    FixedArray<float, 3> com1{0 * meters, 1 * meters, 0 * meters};
    std::shared_ptr<RigidBodyVehicle> r0 = rigid_cuboid("r0", "r0_no_id", mass, size, com0);
    std::shared_ptr<RigidBodyVehicle> r1 = rigid_cuboid("r1", "r1_no_id", mass, size, com1);
    r0->rbp_.abs_com_ = 0;
    r1->rbp_.abs_com_ = com1.casted<double>();
    r0->rbp_.rotation_ = fixed_identity_array<float, 3>();
    r1->rbp_.rotation_ = fixed_identity_array<float, 3>();
    // Hack to get identical values in the following tests.
    r1->rbp_.I_ = r0->rbp_.I_;
    float dt = cfg.dt_substeps();
    r0->rbp_.integrate_delta_v({0.f, -9.8f * meters / (s * s) * dt, 0.f});
    r1->rbp_.integrate_delta_v({0.f, -9.8f * meters / (s * s) * dt, 0.f});
    {
        r0->rbp_.advance_time(dt);
    }
    {
        r1->rbp_.advance_time(dt);
    }
    
    // std::cerr << r0->rbp_.v_ << std::endl;
    // std::cerr << r1->rbp_.v_ << std::endl;
    assert_allclose(r0->rbp_.v_, FixedArray<float, 3>{0.f, -0.163366f * meters / s, 0.f}, (float)1e-12);
    assert_allclose(r1->rbp_.v_, FixedArray<float, 3>{0.f, -0.163366f * meters / s, 0.f}, (float)1e-12);
    r0->integrate_force({{1.2f * meters, 3.4f * meters, 5.6f * meters}, com0.casted<double>() + FixedArray<double, 3>{7.8 * meters, 6.5 * meters, 4.3 * meters}}, cfg);
    r1->integrate_force({{1.2f * meters, 3.4f * meters, 5.6f * meters}, com1.casted<double>() + FixedArray<double, 3>{7.8 * meters, 6.5 * meters, 4.3 * meters}}, cfg);
    {
        r0->rbp_.advance_time(dt);
    }
    {
        r1->rbp_.advance_time(dt);
    }
    assert_allclose(r0->rbp_.v_, r1->rbp_.v_);
    assert_allclose(
        r0->velocity_at_position(com0.casted<double>()),
        r1->velocity_at_position(com1.casted<double>()));
    {
        r0->advance_time(cfg, nullptr);
    }
    {
        r1->advance_time(cfg, nullptr);
    }
    assert_allclose(
        r0->velocity_at_position(com0.casted<double>()),
        r1->velocity_at_position(com1.casted<double>()));
}

void test_magic_formula() {
    {
        MagicFormulaArgmax<float> mf{MagicFormula<float>{}};
        assert_isclose(mf.argmax, 0.04665f);
        assert_isclose(mf(mf.argmax), 1.f);
        assert_isclose(mf(-mf.argmax), -1.f);
        assert_isclose(mf(2 * mf.argmax, MagicFormulaMode::NO_SLIP), 1.f);
        assert_isclose(mf(-2 * mf.argmax, MagicFormulaMode::NO_SLIP), -1.f);
        assert_isclose(mf(0.9f * mf.argmax, MagicFormulaMode::NO_SLIP), 0.997996f);
        assert_isclose(mf(-0.9f * mf.argmax, MagicFormulaMode::NO_SLIP), -0.997996f);
        assert_isclose(mf(2 * mf.argmax), 0.952219f);
    }
    {
        MagicFormulaArgmax<float> mf{MagicFormula<float>{.B = 41 * 0.044f * 8}};
        assert_isclose(mf.argmax, 0.132484f);
    }
    {
        MagicFormulaArgmax<float> mf{MagicFormula<float>{.B = 41 * 0.044f * 10}};
        assert_isclose(mf.argmax, 0.10599f);
    }
}

void test_track_element() {
    TrackElement te{
        .elapsed_seconds = 1,
        .transformations = {
            OffsetAndTaitBryanAngles<float, double, 3>{
                FixedArray<float, 3>{5.f, 6.f, 7.f},
                FixedArray<double, 3>{2., 3., 4.}}}};
    std::stringstream sstr;
    te.write_to_stream(sstr, TransformationMatrix<double, double, 3>::identity());
    TrackElement te2 = TrackElement::from_stream(sstr, TransformationMatrix<double, double, 3>::identity(), te.transformations.size());
    assert_isequal(te.elapsed_seconds, te2.elapsed_seconds);
    assert_allequal(te.transformation().position(), te2.transformation().position());
    assert_allequal(te.transformation().rotation(), te2.transformation().rotation());
}

int main(int argc, char** argv) {
    enable_floating_point_exceptions();

    try {
        test_aim();
        test_power_to_force_negative();
        test_power_to_force_stiction_normal();
        // test_power_to_force_P_normal();
        // test_power_to_force_stiction_tangential();
        test_com();
        test_magic_formula();
        test_track_element();
    } catch (const std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
