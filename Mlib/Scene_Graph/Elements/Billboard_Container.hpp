#pragma once
#include <Mlib/Billboard_Id.hpp>
#include <Mlib/Geometry/Intersection/Axis_Aligned_Bounding_Box.hpp>
#include <Mlib/Geometry/Intersection/Bvh.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <vector>

namespace Mlib {

template <class TPosition>
struct PositionAndYAngleAndBillboardId {
    FixedArray<TPosition, 3> position;
    BillboardId billboard_id;
    SceneDir yangle;
    inline const auto& primitive() const {
        return position;
    }
    inline PositionAndYAngleAndBillboardId& payload() {
        return *this;
    }
    inline const PositionAndYAngleAndBillboardId& payload() const {
        return *this;
    }
};
static_assert(sizeof(PositionAndYAngleAndBillboardId<HalfCompressedScenePos>) == 2 * 3 + 2 + 4);

inline PositionAndYAngleAndBillboardId<CompressedScenePos> operator + (
    const PositionAndYAngleAndBillboardId<HalfCompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    return { a.position.casted<CompressedScenePos>() + reference, a.billboard_id, a.yangle };
}

inline PositionAndYAngleAndBillboardId<HalfCompressedScenePos> operator - (
    const PositionAndYAngleAndBillboardId<CompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    auto p = a.position - reference;
    auto cp = p.casted<HalfCompressedScenePos>();
    if (any(cp.casted<CompressedScenePos>() != p)) {
        THROW_OR_ABORT("PositionAndYAngleAndBillboardId: Could not compress scene position");
    }
    return { cp, a.billboard_id, a.yangle };
}

template <class TPosition>
struct PositionAndBillboardId {
    FixedArray<TPosition, 3> position;
    BillboardId billboard_id;
    inline const auto& primitive() const
    {
        return position;
    }
    inline PositionAndBillboardId& payload() {
        return *this;
    }
    inline const PositionAndBillboardId& payload() const {
        return *this;
    }
};
static_assert(sizeof(PositionAndBillboardId<HalfCompressedScenePos>) == 8);

inline PositionAndBillboardId<CompressedScenePos> operator + (
    const PositionAndBillboardId<HalfCompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    return { a.position.casted<CompressedScenePos>() + reference, a.billboard_id };
}

inline PositionAndBillboardId<HalfCompressedScenePos> operator - (
    const PositionAndBillboardId<CompressedScenePos>& a,
    const FixedArray<CompressedScenePos, 3>& reference)
{
    auto p = a.position - reference;
    auto cp = p.casted<HalfCompressedScenePos>();
    if (any(cp.casted<CompressedScenePos>() != p)) {
        THROW_OR_ABORT("PositionAndBillboardId: Could not compress scene position");
    }
    return { cp, a.billboard_id };
}

class BillboardContainer {
public:
    auto& add(const PositionAndYAngleAndBillboardId<CompressedScenePos>& pyb) {
        if (empty()) {
            reference_point_ = Mlib::center(pyb.primitive());
        }
        return pybs_.emplace_back(pyb - reference_point_);
    }
    auto& add(const PositionAndBillboardId<CompressedScenePos>& pb) {
        if (empty()) {
            reference_point_ = Mlib::center(pb.primitive());
        }
        return pbs_.emplace_back(pb - reference_point_);
    }
    void fill(auto& container) const {
        for (const auto& d : pybs_) {
            container.insert(d + reference_point_);
        }
        for (const auto& d : pbs_) {
            container.insert(d + reference_point_);
        }
    }
    bool visit(const auto& aabb, const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            auto ud = d + reference_point_;
            if (intersects(aabb, ud.primitive())) {
                if (!pyb_visitor(ud.payload())) {
                    return false;
                }
            }
        }
        for (const auto& d : pbs_) {
            auto ud = d + reference_point_;
            if (intersects(aabb, ud.primitive())) {
                if (!pb_visitor(ud.payload())) {
                    return false;
                }
            }
        }
        return true;
    }
    bool visit_all(const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            if (!pyb_visitor(d + reference_point_)) {
                return false;
            }
        }
        for (const auto& d : pbs_) {
            if (!pb_visitor(d + reference_point_)) {
                return false;
            }
        }
        return true;
    }
    bool visit_pairs(const auto& aabb, const auto& pyb_visitor, const auto& pb_visitor) const {
        for (const auto& d : pybs_) {
            auto ud = d + reference_point_;
            if (intersects(aabb, ud.primitive())) {
                if (!pyb_visitor(ud)) {
                    return false;
                }
            }
        }
        for (const auto& d : pbs_) {
            auto ud = d + reference_point_;
            if (intersects(aabb, ud.primitive())) {
                if (!pb_visitor(ud)) {
                    return false;
                }
            }
        }
        return true;
    }
    void print(std::ostream& ostr, size_t rec = 0) const {
        for (const auto& d : pybs_) {
            Mlib::print((d + reference_point_).primitive(), ostr, rec + 1);
        }
        for (const auto& d : pbs_) {
            Mlib::print((d + reference_point_).primitive(), ostr, rec + 1);
        }
    }
    bool empty() const {
        return pybs_.empty() && pbs_.empty();
    }
    size_t size() const {
        return pybs_.size() + pbs_.size();
    }
    void clear() {
        pybs_.clear();
        pbs_.clear();
    }
private:
    FixedArray<CompressedScenePos, 3> reference_point_ = uninitialized;
    std::vector<PositionAndYAngleAndBillboardId<HalfCompressedScenePos>> pybs_;
    std::vector<PositionAndBillboardId<HalfCompressedScenePos>> pbs_;
};

}
