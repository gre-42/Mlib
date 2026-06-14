#include "Vehicle_Location_History_Entry.hpp"

using namespace Mlib;
using namespace Mlib::Vehicle;

AbsoluteVehicleLocation16 VehicleLocation::fixed_point() const {
    return AbsoluteVehicleLocation16{
        .T = T.casted<CompressedSceneT32>(),
        .r = r.casted<CompressedSceneR16>(),
        .v_com = v_com.casted<CompressedSceneV16>(),
        .w = w.casted<CompressedSceneW16>(),
    };
}

VehicleLocation AbsoluteVehicleLocation16::floating_point() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r = r.casted<SceneDir>(),
        .v_com = v_com.casted<SceneDir>(),
        .w = w.casted<SceneDir>(),
    };
}

AbsoluteVehicleLocation8 AbsoluteVehicleLocation16::downsample() const {
    return AbsoluteVehicleLocation8{
        .T = T.casted<CompressedSceneT16>(),
        .r = r.casted<CompressedSceneR8>(),
        .v_com = v_com.casted<CompressedSceneV8>(),
        .w = w.casted<CompressedSceneW8>(),
    };
}

AbsoluteVehicleLocation8 AbsoluteVehicleLocation8::nan() {
    return {
        fixed_full<CompressedSceneT16, 3>((CompressedSceneT16)4.2),
        fixed_full<CompressedSceneR8, 3>((CompressedSceneR8)4.2),
        fixed_full<CompressedSceneV8, 3>((CompressedSceneV8)4.2),
        fixed_full<CompressedSceneW8, 3>((CompressedSceneW8)4.2),
    };
}
VehicleLocation AbsoluteVehicleLocation8::upsample() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r = r.casted<SceneDir>(),
        .v_com = v_com.casted<SceneDir>(),
        .w = w.casted<SceneDir>()
    };
}

DeltaVehicleLocation Mlib::Vehicle::minus_position(const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base, IncrementalConfig& c) {
    return DeltaVehicleLocation{
        .T = a.T.template array_array_binop<DeltaSceneT16>(base.T, [&c](const auto& a, const auto &b){ return minus_position(a, b, c); }),
        .r = a.r.template array_array_binop<DeltaSceneR8>(base.r, [&c](const auto& a, const auto &b){ return minus_angle(a, b, c); }),
        .v_com = a.v_com.template array_array_binop<DeltaSceneV8>(base.v_com, [&c](const auto& a, const auto &b){ return minus_velocity(a, b, c); }),
        .w = a.w.template array_array_binop<DeltaSceneW8>(base.w, [&c](const auto& a, const auto &b){ return minus_angular_velocity(a, b, c); })
    };
}

AbsoluteVehicleLocation16 Mlib::Vehicle::operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b) {
    return AbsoluteVehicleLocation16{
        .T = base.T.template array_array_binop<CompressedSceneT32>(b.T, [](const auto& a, const auto &b){ return plus_position(a, b); }),
        .r = base.r.template array_array_binop<CompressedSceneR16>(b.r, [](const auto& a, const auto &b){ return plus_angle(a, b); }),
        .v_com = base.v_com.template array_array_binop<CompressedSceneV16>(b.v_com, [](const auto& a, const auto &b){ return plus_velocity(a, b); }),
        .w = base.w.template array_array_binop<CompressedSceneW16>(b.w, [](const auto& a, const auto &b){ return plus_angular_velocity(a, b); })
    };
}
