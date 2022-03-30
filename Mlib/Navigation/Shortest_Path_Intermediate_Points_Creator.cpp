#include "Shortest_Path_Intermediate_Points_Creator.hpp"
#include <Mlib/Array/Fixed_Array.hpp>
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
        return std::vector<FixedArray<float, 3>>(++res.begin(), --res.end());
    }
}
