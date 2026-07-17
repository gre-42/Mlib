#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_R.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_T.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_V.hpp>
#include <Mlib/Physics/Incremental_Coordinates/Incremental_Scene_W.hpp>
#include <Mlib/Scene/Remote/Location_History/Velocity_Config.hpp>

namespace Mlib::Vehicle {

struct AbsoluteVehicleLocation8;
struct AbsoluteVehicleLocation16;

struct VehicleLocation {
    FixedArray<ScenePos, 3> T;
    FixedArray<SceneDir, 3> r;
    #ifndef WITHOUT_VELOCITY
    FixedArray<SceneDir, 3> v_com;
    FixedArray<SceneDir, 3> w;
    #endif
    AbsoluteVehicleLocation16 fixed_point() const;
};

struct AbsoluteVehicleLocation16 {
    VehicleLocation floating_point() const;
    AbsoluteVehicleLocation8 downsample() const;
    FixedArray<CompressedSceneT32, 3> T;
    FixedArray<CompressedSceneR16, 3> r;
    #ifndef WITHOUT_VELOCITY
    FixedArray<CompressedSceneV16, 3> v_com;
    FixedArray<CompressedSceneW16, 3> w;
    #endif
};

struct AbsoluteVehicleLocation8 {
    EFixedArray<CompressedSceneT16, 3> T;
    EFixedArray<CompressedSceneR8, 3> r;
    #ifndef WITHOUT_VELOCITY
    EFixedArray<CompressedSceneV8, 3> v_com;
    EFixedArray<CompressedSceneW8, 3> w;
    #endif
    VehicleLocation upsample() const;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(T);
        archive(r);
        #ifndef WITHOUT_VELOCITY
        archive(v_com);
        archive(w);
        #endif
    }
};

struct DeltaVehicleLocation {
    EFixedArray<DeltaSceneT16, 3> T;
    EFixedArray<DeltaSceneR8, 3> r;
    #ifndef WITHOUT_VELOCITY
    EFixedArray<DeltaSceneV8, 3> v_com;
    EFixedArray<DeltaSceneW8, 3> w;
    #endif
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(T);
        archive(r);
        #ifndef WITHOUT_VELOCITY
        archive(v_com);
        archive(w);
        #endif
    }
};

DeltaVehicleLocation minus_position(const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base, IncrementalConfig& c);
AbsoluteVehicleLocation16 operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b);

}
