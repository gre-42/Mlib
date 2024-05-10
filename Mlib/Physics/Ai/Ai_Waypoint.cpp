#include "Ai_Waypoint.hpp"
#include <Mlib/Geometry/Intersection/Ray_Sphere_Intersection.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Geometry/Ray_Segment_3D.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

FixedArray<double, 3> AiWaypoint::interpolated_position(
	const FixedArray<double, 3>& vehicle_position,
	double radius_squared) const
{
	if (!position_of_destination.has_value()) {
		THROW_OR_ABORT("Position of desination is undefined");
	}
	auto& pod = position_of_destination.value();
	if ((waypoint_history == nullptr) ||
		waypoint_history->empty())
	{
		return pod;
	}
	RaySegment3D<double> rs(waypoint_history->rbegin()->position, pod);
	double lambda;
	if (!ray_intersects_sphere(
		rs.start,
		rs.direction,
		vehicle_position,
		radius_squared,
		&lambda,
		(double*)nullptr))
	{
		return pod;
	} else {
		if ((lambda >= 0) && (lambda < rs.length)) {
			return rs.start + rs.direction * lambda;
		} else {
			return pod;
		}
	}
	return pod;
}
