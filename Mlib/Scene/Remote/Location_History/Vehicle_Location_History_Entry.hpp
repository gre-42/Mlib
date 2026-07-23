#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_R.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_T.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_V.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_W.hpp>

namespace Mlib::Vehicle {

struct AbsoluteVehicleLocation8;
struct AbsoluteVehicleLocation16;

struct VehicleLocation {
    FixedArray<ScenePos, 3> T;
    FixedArray<SceneDir, 3> r;
    AbsoluteVehicleLocation16 fixed_point() const;
};

struct AbsoluteVehicleLocation16 {
    VehicleLocation floating_point() const;
    AbsoluteVehicleLocation8 downsample() const;
    FixedArray<CompressedSceneT32, 3> T;
    FixedArray<CompressedSceneR16, 3> r;
};

struct AbsoluteVehicleLocation8 {
    EFixedArray<CompressedSceneT16, 3> T;
    EFixedArray<CompressedSceneR8, 3> r;
    VehicleLocation upsample() const;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(T);
        archive(r);
    }
};

struct DeltaVehicleLocation {
    EFixedArray<DeltaSceneT16, 3> T;
    EFixedArray<DeltaSceneR8, 3> r;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(T);
        archive(r);
    }
};

DeltaVehicleLocation minus_position(const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base, IncrementalConfig& c);
AbsoluteVehicleLocation16 operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b);

}
