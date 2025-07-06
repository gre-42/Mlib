#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/Math/Fixed_Rodrigues.hpp>
#include <Mlib/Math/Fixed_Scaled_Unit_Vector.hpp>
#include <Mlib/Math/Fixed_Test.hpp>
#include <Mlib/Math/Pi.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Physics/Collision/Magic_Formula.hpp>
#include <Mlib/Physics/Collision/Power_To_Force.hpp>
#include <Mlib/Physics/Misc/Aim.hpp>
#include <Mlib/Physics/Misc/Beacon.hpp>
#include <Mlib/Physics/Misc/Gravity_Efp.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Engine.hpp>
#include <Mlib/Physics/Physics_Engine/Physics_Phase.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Body_Vehicle.hpp>
#include <Mlib/Physics/Rigid_Body/Rigid_Primitives.hpp>
#include <Mlib/Scene_Graph/Instances/Static_World.hpp>
#include <Mlib/Signal/Pid_Controller.hpp>
#include <Mlib/Stats/Linspace.hpp>

using namespace Mlib;

void test_aim() {
    {
        FixedArray<ScenePos, 3> gun_pos{1.f, 2.f, 3.f};
        FixedArray<ScenePos, 3> target_pos{4.f, 2.f, 2.f};
        {
            ScenePos velocity = 20;
            ScenePos gravity = 9.8f;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7f, 10};
            assert_isclose<ScenePos>(aim.angle, 0.0387768f);
            assert_isclose<ScenePos>(aim.aim_offset, 0.122684f);
        }
        {
            ScenePos velocity = 10;
            ScenePos gravity = 9.8f;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7f, 10};
            assert_isclose<ScenePos>(aim.angle, 0.157546f);
            assert_isclose<ScenePos>(aim.aim_offset, 0.502367f);
        }
    }
    {
        {
            FixedArray<ScenePos, 3> gun_pos{1.f, 2.f, 3.f};
            FixedArray<ScenePos, 3> target_pos{4.f, 2.f, 3.f};
            ScenePos velocity = 10;
            ScenePos gravity = 9.8f;
            Aim aim{gun_pos, target_pos, 0, velocity, gravity, 1e-7f, 10};
            assert_isclose<ScenePos>(aim.angle, 0.149205f);
            assert_isclose<ScenePos>(aim.aim_offset, 0.450965f);
        }
        {
            FixedArray<ScenePos, 3> gun_pos{1.f, 2.f, 3.f};
            FixedArray<ScenePos, 3> target_pos{5.f, 2.f, 3.f};
            ScenePos velocity = 10;
            ScenePos gravity = 9.8f;
            Aim aim{gun_pos, target_pos, 1, velocity, gravity, 1e-7f, 10};
            assert_isclose<ScenePos>(aim.angle, 0.111633f);
            assert_isclose<ScenePos>(aim.aim_offset, 0.448397f);
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
        auto F = power_to_force_infinite_mass(1e4f, 1e-1f, 5e4f, 5e4f, INFINITY, n3, P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // lerr() << v3;
    }
    assert_isclose<float>(v3(0), 32.4703f, 1e-4f);
    for (float t = 0; t < 10; t += dt) {
        auto F = power_to_force_infinite_mass(1e4f, 1e-1f, 5e4f, 5e4f, INFINITY, n3, -P, v3, dt, alpha0, false);
        v3 += F / m * dt;
        // lerr() << v3;
    }
    assert_isclose<float>(v3(0), -26.8054f, 1e-4f);
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
        auto F = power_to_force_infinite_mass(1e4f, 1e-1f, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        F += power_to_force_infinite_mass(1e4f, 1e-1f, g * m * stiction_coefficient / 2, 1e3, INFINITY, n3, P, v3, dt, alpha0, true);
        v3 += F / m * dt;
    }
    assert_isclose<float>(v3(0), 97.0226f, 1e-4f);
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
    const auto identity = TransformationMatrix<double, double, 3>::identity();
    const auto gravity = FixedScaledUnitVector<float, 3>{ { 0.f, -9.8f, 0.f } };
    StaticWorld world{
        .geographic_mapping = &identity,
        .inverse_geographic_mapping = &identity,
        .gravity = &gravity,
        .wind = nullptr,
        .time = std::chrono::steady_clock::now()
    };

    RigidBodies rbs{ cfg };
    float mass = 123.f * kg;
    FixedArray<float, 3> size{2 * meters, 3 * meters, 4 * meters};
    FixedArray<float, 3> com0{0 * meters, 0 * meters, 0 * meters};
    FixedArray<float, 3> com1{0 * meters, 1 * meters, 0 * meters};
    auto r0 = rigid_cuboid(global_object_pool, "r0", "r0_no_id", mass, size, com0);
    auto r1 = rigid_cuboid(global_object_pool, "r1", "r1_no_id", mass, size, com1);
    r0->rbp_.abs_com_ = 0;
    r1->rbp_.abs_com_ = com1.casted<ScenePos>();
    r0->rbp_.rotation_ = fixed_identity_array<float, 3>();
    r1->rbp_.rotation_ = fixed_identity_array<float, 3>();
    // Hack to get identical values in the following tests.
    r1->rbp_.I_ = r0->rbp_.I_;
    CollisionGroup group{
        .penetration_class = PenetrationClass::STANDARD,
        .nsubsteps = cfg.nsubsteps,
        .divider = 1,
    };
    PhysicsPhase phase{
        .burn_in = false,
        .substep = 0,
        .group = group,
    };
    float dt = cfg.dt_substeps(phase);
    r0->rbp_.integrate_delta_v({0.f, -9.8f * meters / (seconds * seconds) * dt, 0.f}, dt);
    r1->rbp_.integrate_delta_v({0.f, -9.8f * meters / (seconds * seconds) * dt, 0.f}, dt);
    {
        r0->rbp_.advance_time(dt);
    }
    {
        r1->rbp_.advance_time(dt);
    }
    
    // lerr() << r0->rbp_.v_;
    // lerr() << r1->rbp_.v_;
    assert_allclose(r0->rbp_.v_com_, FixedArray<float, 3>{0.f, -0.163366f * meters / seconds, 0.f}, (float)1e-12);
    assert_allclose(r1->rbp_.v_com_, FixedArray<float, 3>{0.f, -0.163366f * meters / seconds, 0.f}, (float)1e-12);
    auto dx = FixedArray<ScenePos, 3>{7.8f * meters, 6.5f * meters, 4.3f * meters};
    r0->integrate_force({{1.2f * meters, 3.4f * meters, 5.6f * meters}, com0.casted<ScenePos>() + dx}, cfg, phase);
    r1->integrate_force({{1.2f * meters, 3.4f * meters, 5.6f * meters}, com1.casted<ScenePos>() + dx}, cfg, phase);
    {
        r0->rbp_.advance_time(dt);
    }
    {
        r1->rbp_.advance_time(dt);
    }
    assert_allclose(r0->rbp_.v_com_, r1->rbp_.v_com_);
    assert_allclose(
        r0->velocity_at_position(com0.casted<ScenePos>()),
        r1->velocity_at_position(com1.casted<ScenePos>()));
    {
        r0->advance_time(cfg, world, nullptr, phase);
    }
    {
        r1->advance_time(cfg, world, nullptr, phase);
    }
    assert_allclose(
        r0->velocity_at_position(com0.casted<ScenePos>()),
        r1->velocity_at_position(com1.casted<ScenePos>()));
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
            UOffsetAndTaitBryanAngles<float, ScenePos, 3>{
                FixedArray<float, 3>{5.f, 6.f, 7.f},
                FixedArray<ScenePos, 3>{2.f, 3.f, 4.f}}}};
    std::stringstream sstr;
    te.write_to_stream(sstr, TransformationMatrix<double, double, 3>::identity());
    TrackElement te2 = TrackElement::from_stream(sstr, TransformationMatrix<double, double, 3>::identity(), te.transformations.size());
    assert_isequal(te.elapsed_seconds, te2.elapsed_seconds);
    assert_allequal(te.transformation().position, te2.transformation().position);
    assert_allequal(te.transformation().rotation, te2.transformation().rotation);
}

void test_pid() {
    PidController<float, float> pid{ 2.f, 5.f, 7.f, 0.2f };
    auto pid2 = pid.changed_time_step(1.f / 60, 3.f / 60);
    for (size_t i = 0; i < 10; ++i) {
        pid2(5.f);
        pid2(5.f);
        linfo() << std::abs(pid(5.f) - pid2(5.f));
    }
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
        test_pid();
    } catch (const std::runtime_error& e) {
        lerr() << e.what();
        return 1;
    }
    return 0;
}
