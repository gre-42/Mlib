#include <Mlib/Geometry/Mesh/Load_Obj.hpp>
#include <Mlib/Images/Draw_Bmp.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Misc/Rigid_Body.hpp>
#include <Mlib/Physics/Misc/Rigid_Primitives.hpp>
#include <Mlib/Physics/Misc/Tracking_Wheel.hpp>
#include <Mlib/Physics/Physics_Engine.hpp>
#include <Mlib/Stats/Linspace.hpp>
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
    float alpha0 = 0.2;
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(1e4, 1e-1, 5e4, 5e4, INFINITY, n3, P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), 32.4703, 1e-4);
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(1e4, 1e-1, 5e4, 5e4, INFINITY, n3, -P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // std::cerr << v3 << std::endl;
    }
    assert_isclose<float>(v3(0), -26.8054, 1e-4);
}

void test_power_to_force_stiction_normal() {
    FixedArray<float, 3> n3{1, 0, 0};
    float P = INFINITY;
    float g = 9.8;
    FixedArray<float, 3> v3{0, 0, 0};
    float dt = 0.016667;
    float m = 1000;
    float stiction_coefficient = 1;
    float alpha0 = 0.2;
    for(float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(1e4, 1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        F += power_to_force_infinite_mass(1e4, 1e-1, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        v3 += F / m * dt;
    }
    assert_isclose<float>(v3(0), 97.0226, 1e-4);
}
// Infinite max_stiction_force no longer supported
//
// void test_power_to_force_P_normal() {
//     FixedArray<float, 3> n3{1, 0, 0};
//     float P = 51484.9; // Watt, 70 PS
//     FixedArray<float, 3> v3{0, 0, 0};
//     float dt = 0.016667;
//     float m = 1000;
//     for(float t = 0; t < 10; t += dt) {
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
//     float dt = 0.016667;
//     float m = 1000;
//     float stiction_coefficient = 1;
//     auto get_force = [&](float v){
//         FixedArray<float, 3> v3{0, v, 0};
//         return power_to_force_infinite_mass(10, 20, 1e-1, g * m * stiction_coefficient, 1e3, INFINITY, n3, P, 4321, v3, dt, true);
//     };
//     Array<float> vs = linspace(0.f, 5 / 3.6f, 100.f);
//     Array<float> fs{ArrayShape{0}};
//     for(float v : vs.flat_iterable()) {
//         fs.append(get_force(v)(1));
//     }
//     Array<float>({vs, fs}).T().save_txt_2d("vf.m");
//     // assert_isclose<float>(get_force(1.2)(1), 97.0226, 1e-4);
// }

void test_com() {
    PhysicsEngineConfig cfg;
    cfg.damping = 0;
    cfg.friction = 0;

    RigidBodies rbs{cfg};
    float mass = 123;
    FixedArray<float, 3> size{2, 3, 4};
    FixedArray<float, 3> com0{0, 0, 0};
    FixedArray<float, 3> com1{0, 1, 0};
    std::shared_ptr<RigidBody> r0 = rigid_cuboid(rbs, mass, size, com0);
    std::shared_ptr<RigidBody> r1 = rigid_cuboid(rbs, mass, size, com1);
    r0->rbi_.rbp_.abs_com_ = 0;
    r1->rbi_.rbp_.abs_com_ = com1;
    r0->rbi_.rbp_.rotation_ = fixed_identity_array<float, 3>();
    r1->rbi_.rbp_.rotation_ = fixed_identity_array<float, 3>();
    // Hack to get identical values in the following tests.
    r1->rbi_.rbp_.I_ = r0->rbi_.rbp_.I_;
    r0->integrate_gravity({0, -9.8, 0});
    r1->integrate_gravity({0, -9.8, 0});
    {
        r0->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    {
        r1->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    
    // std::cerr << r0->rbi_.rbp_.v_ << std::endl;
    // std::cerr << r1->rbi_.rbp_.v_ << std::endl;
    assert_allclose(r0->rbi_.rbp_.v_.to_array(), Array<float>{0, -0.163366, 0}, 1e-12);
    assert_allclose(r1->rbi_.rbp_.v_.to_array(), Array<float>{0, -0.163366, 0}, 1e-12);
    r0->integrate_force({{1.2f, 3.4f, 5.6f}, com0 + FixedArray<float, 3>{7.8f, 6.5f, 4.3f}});
    r1->integrate_force({{1.2f, 3.4f, 5.6f}, com1 + FixedArray<float, 3>{7.8f, 6.5f, 4.3f}});
    {
        r0->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    {
        r1->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    assert_allclose(r0->rbi_.rbp_.v_.to_array(), r1->rbi_.rbp_.v_.to_array());
    assert_allclose(r0->rbi_.a_.to_array(), r1->rbi_.a_.to_array());
    assert_allclose(r0->rbi_.T_.to_array(), r1->rbi_.T_.to_array());
    assert_allclose(
        r0->velocity_at_position(com0).to_array(),
        r1->velocity_at_position(com1).to_array());
    {
        r0->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    {
        r1->advance_time(cfg.dt, cfg.min_acceleration, cfg.min_velocity, cfg.min_angular_velocity, cfg.physics_type, cfg.resolve_collision_type, cfg.hand_break_velocity, nullptr);
    }
    assert_allclose(
        r0->velocity_at_position(com0).to_array(),
        r1->velocity_at_position(com1).to_array());
}

void test_sticky_spring() {
    FixedArray<float, 3> position{5, 3, 2.6};
    float spring_constant = 2;
    float stiction_force = 1.23;
    float friction_force = 1.12;
    FixedArray<float, 3> force;
    bool slipping;
    StickySpring s{
        .point_of_contact = {1, 2, 3},
        .pid = {1, 0, 0, 0}
    };
    s.update_position(
        position,
        spring_constant,
        stiction_force,
        friction_force,
        nullptr,
        true,  // move_point_of_contact
        force,
        slipping);
    assert_allclose(
        s.point_of_contact.to_array(),
        Array<float>{4.40615, 2.85154, 2.65938},
        1e-5);
}

void test_tracking_wheel() {
    FixedArray<float, 3> rotation_axis{1, 0, 0};
    FixedArray<float, 3> power_axis{0, 0, 1};
    FixedArray<float, 3> velocity{0, 0, 1.2};
    size_t ntires = 10;
    float max_dist = 0.1;
    float radius = 2;
    float dt = 1.f / 60;
    TrackingWheel tw{rotation_axis, radius, ntires, max_dist, dt};
    float spring_constant = 3e4;
    float stiction_force = 1e3;
    float friction_force = 1e2;
    {
        FixedArray<float, 3, 3> rotation = rodrigues<float>({0, 1, 0}, 0.f);
        FixedArray<float, 3> translation = {0.f, 0.f, 0.f};
        tw.set_w(1.23);
        tw.notify_intersection(rotation, translation, {0, -1, 0}, {1, 0, 0}, stiction_force, friction_force);
        {
            RigidBodyIntegrator rbi = rigid_cuboid_integrator(1e3, {1.f, 2.f, 3.f}, {0.f, 0.f, 0.5f});
            float power_internal;
            float power_external;
            float moment;
            bool slipping;
            tw.update_position(rotation, translation, power_axis, velocity, spring_constant, dt, rbi, power_internal, power_external, moment, slipping, nullptr);
            assert_allclose(rbi.a_.to_array(), Array<float>{0, 0, 0});
            tw.notify_intersection(rotation, translation, {0, -1, 0}, {1, 0, 0}, stiction_force, friction_force);
            tw.update_position(rotation, translation, power_axis, velocity, spring_constant, dt, rbi, power_internal, power_external, moment, slipping, nullptr);
            assert_allclose(rbi.a_.to_array(), Array<float>{0, 0, -0.861});
        }
    }
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
        MagicFormulaArgmax<float> mf{MagicFormula<float>{.B = 41 * 0.044 * 8}};
        assert_isclose(mf.argmax, 0.132484f);
    }
    {
        MagicFormulaArgmax<float> mf{MagicFormula<float>{.B = 41 * 0.044 * 10}};
        assert_isclose(mf.argmax, 0.10599f);
    }
}

int main(int argc, const char** argv) {
    #ifndef __MINGW32__
    feenableexcept(FE_INVALID);
    #endif

    test_aim();
    test_power_to_force_negative();
    test_power_to_force_stiction_normal();
    // test_power_to_force_P_normal();
    // test_power_to_force_stiction_tangential();
    test_com();
    test_sticky_spring();
    test_tracking_wheel();
    test_magic_formula();
    return 0;
}
