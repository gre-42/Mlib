#include "Shortest_Path_Intermediate_Points_Creator.hpp"
#include <Mlib/Geometry/Exceptions/Edge_Exception.hpp>
#include <Mlib/Geometry/Exceptions/Point_Exception.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

ShortestPathIntermediatePointsCreator::ShortestPathIntermediatePointsCreator(
    const Sample_SoloMesh& ssm,
    const std::map<OrderableFixedArray<float, 3>, dtPolyRef>& poly_refs,
    float step_size)
    : ssm_{ ssm }
    , poly_refs_{ poly_refs }
    , step_size_{ step_size }
{}

UUVector<FixedArray<float, 3>> ShortestPathIntermediatePointsCreator::operator () (
    const FixedArray<float, 3>& p0,
    const FixedArray<float, 3>& p1,
    const float& distance) const
{
    auto lp0_it = poly_refs_.find(OrderableFixedArray{p0});
    if (lp0_it == poly_refs_.end()) {
        THROW_OR_ABORT2((PointException{ p0, "Could not find poly for start" }));
    }
    auto lp1_it = poly_refs_.find(OrderableFixedArray{p1});
    if (lp1_it == poly_refs_.end()) {
        THROW_OR_ABORT2((PointException{ p1, "Could not find poly for end" }));
    }
    auto res = ssm_.shortest_path(
        LocalizedNavmeshNode{
            .position = p0,
            .polyRef = lp0_it->second},
        LocalizedNavmeshNode{
            .position = p1,
            .polyRef = lp1_it->second},
        step_size_);
    if (res.empty()) {
        THROW_OR_ABORT("Empty shortest path");
    } else if (res.size() == 1) {
        // THROW_OR_ABORT2((EdgeException{p0, p1, "Unexpected path length"}));
        lwarn() << "Shortest path consists of a single point, probably due to duplicate points";
        return {};
    } else if (res.size() == 2) {
        return {};
    } else {
        return UUVector<FixedArray<float, 3>>(++res.begin(), --res.end());
    }
}

UUVector<FixedArray<CompressedScenePos, 3>> ShortestPathIntermediatePointsCreator::operator () (
    const FixedArray<CompressedScenePos, 3>& p0,
    const FixedArray<CompressedScenePos, 3>& p1,
    const CompressedScenePos& distance) const
{
    try {
        auto sresult = (*this)(p0.casted<float>(), p1.casted<float>(), (float)distance);
        UUVector<FixedArray<CompressedScenePos, 3>> result;
        result.reserve(sresult.size());
        for (const auto& v : sresult) {
            result.push_back(v.casted<CompressedScenePos>());
        }
        return result;
    } catch (const EdgeException<float>& e) {
        THROW_OR_ABORT2((EdgeException<CompressedScenePos>{e.a.casted<CompressedScenePos>(), e.b.casted<CompressedScenePos>(), e.what()}));
    } catch (const PointException<float, 3>& e) {
        THROW_OR_ABORT2((PointException<CompressedScenePos, 3>{e.point.casted<CompressedScenePos>(), e.what()}));
    }
}
