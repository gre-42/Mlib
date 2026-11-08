#pragma once
#include <Mlib/Geometry/Primitives/Bvh.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Serialize/Serialize.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <cstdint>

namespace Mlib {

class StreetBvh3d {
    using Triangle3d = FixedArray<CompressedScenePos, 3, 3>;
public:
    explicit StreetBvh3d();
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        if constexpr (Archive::is_saving::value) {
            archive(integral_cast<uint32_t>(bvh.size()));
            bvh.visit_all([&](const auto& d){
                archive(d.payload());
                return true;
            });
        } else {
            uint32_t nelements;
            archive(nelements);
            for (uint32_t i = 0; i < nelements; ++i) {
                Triangle3d tri = uninitialized;
                archive(tri);
                bvh.insert(AxisAlignedBoundingBox<CompressedScenePos, 3>::from_points(tri), tri);
            }
        }
    }
    Bvh<CompressedScenePos, 3, Triangle3d> bvh;
};

}
