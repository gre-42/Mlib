#include "Avatar_Location_History_Entry.hpp"

using namespace Mlib;
using namespace Mlib::Avatar;

AbsoluteVehicleLocation16 VehicleLocation::fixed_point() const {
    return AbsoluteVehicleLocation16{
        .T = T.casted<CompressedSceneT32>(),
        .r1 = fixed_point_angle(r1),
        #ifndef WITHOUT_VELOCITY
        .v_com = v_com.casted<CompressedSceneV16>(),
        .w1 = (CompressedSceneW16)w1,
        #endif
    };
}

VehicleLocation AbsoluteVehicleLocation16::floating_point() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r1 = (SceneDir)r1,
        #ifndef WITHOUT_VELOCITY
        .v_com = v_com.casted<SceneDir>(),
        .w1 = (SceneDir)w1,
        #endif
    };
}

AbsoluteVehicleLocation8 AbsoluteVehicleLocation16::downsample() const {
    return AbsoluteVehicleLocation8{
        .T = T.casted<CompressedSceneT16>(),
        .r1 = (CompressedSceneR8)r1,
        #ifndef WITHOUT_VELOCITY
        .v_com = v_com.casted<CompressedSceneV8>(),
        .w1 = (CompressedSceneW8)w1,
        #endif
    };
}

VehicleLocation AbsoluteVehicleLocation8::upsample() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r1 = (SceneDir)r1,
        #ifndef WITHOUT_VELOCITY
        .v_com = v_com.casted<SceneDir>(),
        .w1 = (SceneDir)w1,
        #endif
    };
}

DeltaVehicleLocation Mlib::Avatar::minus_position(const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base, IncrementalConfig& c) {
    return DeltaVehicleLocation{
        .T = a.T.template array_array_binop<DeltaSceneT16>(base.T, [&c](const auto& a, const auto &b){ return minus_position(a, b, c); }),
        .r1 = minus_angle(a.r1, base.r1, c),
        #ifndef WITHOUT_VELOCITY
        .v_com = a.v_com.template array_array_binop<DeltaSceneV8>(base.v_com, [&c](const auto& a, const auto &b){ return minus_velocity(a, b, c); }),
        .w1 = minus_angular_velocity(a.w1, base.w1, c),
        #endif
    };
}

AbsoluteVehicleLocation16 Mlib::Avatar::operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b) {
    return AbsoluteVehicleLocation16{
        .T = base.T.template array_array_binop<CompressedSceneT32>(b.T, [](const auto& a, const auto &b){ return plus_position(a, b); }),
        .r1 = plus_angle(base.r1, b.r1),
        #ifndef WITHOUT_VELOCITY
        .v_com = base.v_com.template array_array_binop<CompressedSceneV16>(b.v_com, [](const auto& a, const auto &b){ return plus_velocity(a, b); }),
        .w1 = plus_angular_velocity(base.w1, b.w1)
        #endif
    };
}
