#include "Shortest_Path_Intermediate_Points_Creator.hpp"
#include <Mlib/Geometry/Mesh/Edge_Exception.hpp>
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Math/Orderable_Fixed_Array.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>
#include <Mlib/Stats/Min_Max.hpp>

using namespace Mlib;

ShortestPathIntermediatePointsCreator::ShortestPathIntermediatePointsCreator(
    const Sample_SoloMesh& ssm,
    const std::map<OrderableFixedArray<float, 3>, dtPolyRef>& poly_refs,
    float step_size)
: ssm_{ ssm },
  poly_refs_{ poly_refs },
  step_size_{ step_size }
{}

std::vector<FixedArray<float, 3>> ShortestPathIntermediatePointsCreator::operator () (
    const FixedArray<float, 3>& p0,
    const FixedArray<float, 3>& p1,
    const float& distance) const
{
    auto lp0_it = poly_refs_.find(OrderableFixedArray{p0});
    if (lp0_it == poly_refs_.end()) {
        throw std::runtime_error("Could not find poly for start");
    }
    auto lp1_it = poly_refs_.find(OrderableFixedArray{p1});
    if (lp1_it == poly_refs_.end()) {
        throw std::runtime_error("Could not find poly for end");
    }
    auto res = ssm_.shortest_path(
        LocalizedNavmeshNode{
            .position = p0,
            .polyRef = lp0_it->second},
        LocalizedNavmeshNode{
            .position = p1,
            .polyRef = lp1_it->second},
        step_size_);
    if (res.size() < 2) {
        throw EdgeException{p0, p1, "Unexpected path length"};
    } else if (res.size() == 2) {
        return {};
    } else {
        return std::vector<FixedArray<float, 3>>(++res.begin(), --res.end());
    }
}

std::vector<FixedArray<double, 3>> ShortestPathIntermediatePointsCreator::operator () (
    const FixedArray<double, 3>& p0,
    const FixedArray<double, 3>& p1,
    const double& distance) const
{
    try {
        auto sresult = (*this)(p0.casted<float>(), p1.casted<float>(), (float)distance);
        std::vector<FixedArray<double, 3>> result;
        result.reserve(sresult.size());
        for (const auto& v : sresult) {
            result.push_back(v.casted<double>());
        }
        return result;
    } catch (const EdgeException<float>& e) {
        throw EdgeException<double>{e.a.casted<double>(), e.b.casted<double>(), e.what()};
    }
}
