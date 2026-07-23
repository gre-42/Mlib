#include "Vehicle_Location_History_Entry.hpp"

using namespace Mlib;
using namespace Mlib::Vehicle;

AbsoluteVehicleLocation16 VehicleLocation::fixed_point() const {
    return AbsoluteVehicleLocation16{
        .T = T.casted<CompressedSceneT32>(),
        .r = r.template applied<CompressedSceneR16>([](float a){ return fixed_point_angle(a); }),
    };
}

VehicleLocation AbsoluteVehicleLocation16::floating_point() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r = r.casted<SceneDir>(),
    };
}

AbsoluteVehicleLocation8 AbsoluteVehicleLocation16::downsample() const {
    return AbsoluteVehicleLocation8{
        .T = T.casted<CompressedSceneT16>(),
        .r = r.casted<CompressedSceneR8>(),
    };
}

VehicleLocation AbsoluteVehicleLocation8::upsample() const {
    return VehicleLocation{
        .T = T.casted<ScenePos>(),
        .r = r.casted<SceneDir>(),
    };
}

DeltaVehicleLocation Mlib::Vehicle::minus_position(const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base, IncrementalConfig& c) {
    return DeltaVehicleLocation{
        .T = a.T.template array_array_binop<DeltaSceneT16>(base.T, [&c](const auto& a, const auto &b){ return minus_position(a, b, c); }),
        .r = a.r.template array_array_binop<DeltaSceneR8>(base.r, [&c](const auto& a, const auto &b){ return minus_angle(a, b, c); }),
    };
}

AbsoluteVehicleLocation16 Mlib::Vehicle::operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b) {
    return AbsoluteVehicleLocation16{
        .T = base.T.template array_array_binop<CompressedSceneT32>(b.T, [](const auto& a, const auto &b){ return plus_position(a, b); }),
        .r = base.r.template array_array_binop<CompressedSceneR16>(b.r, [](const auto& a, const auto &b){ return plus_angle(a, b); }),
    };
}
