#include "Shortest_Path_Intermediate_Points_Creator.hpp"
#include <Mlib/Math/Fixed_Math.hpp>
#include <Mlib/Stats/Min_Max.hpp>
#include <Mlib/Geometry/Mesh/Edge_Exception.hpp>
#include <Mlib/Navigation/Sample_SoloMesh.hpp>

using namespace Mlib;

ShortestPathIntermediatePointsCreator::ShortestPathIntermediatePointsCreator(const Sample_SoloMesh& ssm)
: ssm_{ ssm }
{}

std::vector<FixedArray<float, 3>> ShortestPathIntermediatePointsCreator::operator () (
    const FixedArray<float, 3>& p0,
    const FixedArray<float, 3>& p1,
    const float& distance) const
{
    auto res = ssm_.shortest_path(p0, p1);
    if (res.size() < 2) {
        throw EdgeException{p0, p1, "Unexpected path length"};
    } else if (res.size() == 2) {
        return {};
    } else {
        if (max(abs(*res.begin() - p0)) > float{1e-6}) {
            throw EdgeException{p0, *res.begin(), "Start point changed its position"};
        }
        if (max(abs(*(--res.end()) - p1)) > float{1e-6}) {
            throw EdgeException{p1, *(--res.end()), "End point changed its position"};
        }
        return std::vector<FixedArray<float, 3>>(++res.begin(), --res.end());
    }
}
