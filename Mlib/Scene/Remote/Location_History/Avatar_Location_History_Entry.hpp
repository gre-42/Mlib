#pragma once
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Misc/Object.hpp>
#include <Mlib/Scene_Config/Incremental_Scene_R.hpp>
#include <Mlib/Scene_Config/Incremental_Scene_T.hpp>
#include <Mlib/Scene_Config/Incremental_Scene_V.hpp>
#include <Mlib/Scene_Config/Incremental_Scene_W.hpp>

namespace Mlib::Avatar {

struct AbsoluteVehicleLocation8;
struct AbsoluteVehicleLocation16;

struct VehicleLocation {
    FixedArray<ScenePos, 3> T;
    SceneDir r1;
    FixedArray<SceneDir, 3> v_com;
    SceneDir w1;
    AbsoluteVehicleLocation16 fixed_point() const;
};

struct AbsoluteVehicleLocation16 {
    VehicleLocation floating_point() const;
    AbsoluteVehicleLocation8 downsample() const;
    FixedArray<CompressedSceneT32, 3> T;
    CompressedSceneR16 r1;
    FixedArray<CompressedSceneV16, 3> v_com;
    CompressedSceneW16 w1;
};

struct AbsoluteVehicleLocation8 {
    EFixedArray<CompressedSceneT16, 3> T;
    CompressedSceneR8 r1;
    EFixedArray<CompressedSceneV8, 3> v_com;
    CompressedSceneW8 w1;
    static AbsoluteVehicleLocation8 nan();
    VehicleLocation upsample() const;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(T);
        archive(r1);
        archive(v_com);
        archive(w1);
    }
};

struct DeltaVehicleLocation {
    EFixedArray<DeltaSceneT16, 3> T;
    DeltaSceneR8 r1;
    EFixedArray<DeltaSceneV8, 3> v_com;
    DeltaSceneW8 w1;
    template <class Archive>
    void serialize(Archive& archive) {
        archive(T);
        archive(r1);
        archive(v_com);
        archive(w1);
    }
};

DeltaVehicleLocation operator - (const AbsoluteVehicleLocation16& a, const AbsoluteVehicleLocation8& base);
AbsoluteVehicleLocation16 operator + (const AbsoluteVehicleLocation8& base, const DeltaVehicleLocation& b);

}
