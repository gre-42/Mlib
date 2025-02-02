#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Geometry/Mesh/Point_And_Flags.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <chrono>
#include <optional>
#include <string>

namespace Mlib {

class Player;
class PathfindingWaypoints;
enum class WayPointLocation;
class SkillMap;
struct StaticWorld;

class SingleWaypoint {
    SingleWaypoint(const SingleWaypoint&) = delete;
    SingleWaypoint& operator = (const SingleWaypoint&) = delete;
public:
    using WayPoint = PointAndFlags<FixedArray<CompressedScenePos, 3>, WayPointLocation>;

    explicit SingleWaypoint(const DanglingBaseClassRef<Player>& player);
    ~SingleWaypoint();
    void move_to_waypoint(const SkillMap& skills, const StaticWorld& world);
    void draw_waypoint_history(const std::string& filename) const;
    void set_waypoint(const WayPoint& waypoint, size_t waypoint_id);
    void set_waypoint(const WayPoint& waypoint);
    void clear_waypoint();
    void set_target_velocity(float v);
    void notify_set_waypoints(size_t nwaypoints);
    void notify_spawn();
    bool has_waypoint() const;
    const WayPoint& get_waypoint() const;
    bool waypoint_reached() const;
    size_t nwaypoints_reached() const;
    size_t target_waypoint_id() const;
    size_t previous_waypoint_id() const;
    std::chrono::steady_clock::time_point last_visited(size_t waypoint_id) const;
private:
    void set_waypoint_internal(
        const std::optional<WayPoint>& waypoint,
        size_t waypoint_id);
    DanglingBaseClassRef<Player> player_;
    float target_velocity_;
    std::optional<WayPoint> waypoint_;
    size_t waypoint_id_;
    size_t previous_waypoint_id_;
    bool waypoint_reached_;
    size_t nwaypoints_reached_;
    std::vector<std::chrono::steady_clock::time_point> last_visited_;
    std::list<WayPoint> waypoint_history_;
    size_t max_waypoint_history_length_;
};

}
