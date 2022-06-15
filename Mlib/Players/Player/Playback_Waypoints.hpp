#pragma once
#include <list>
#include <string>

namespace Mlib {

template <class TDir, class TPos, size_t n>
class TransformationMatrix;
struct TrackElement;
class Player;

class PlaybackWaypoints{
public:
    explicit PlaybackWaypoints(Player& player);
    ~PlaybackWaypoints();
    bool has_waypoints() const;
    void select_next_waypoint();
    void set_waypoints(
        const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
        const std::string& playback_filename,
        float speedup);
private:
    Player& player_;
    std::list<TrackElement> track_;
    std::list<TrackElement>::const_iterator current_track_element_;
    float speedup_;
};

}
