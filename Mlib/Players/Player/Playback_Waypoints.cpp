#include "Playback_Waypoints.hpp"
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <fstream>

using namespace Mlib;

PlaybackWaypoints::PlaybackWaypoints(Player& player)
: player_{player},
  current_track_element_{track_.end()}
{}

PlaybackWaypoints::~PlaybackWaypoints()
{}

bool PlaybackWaypoints::has_waypoints() const {
    return !track_.empty();
}

void PlaybackWaypoints::select_next_waypoint() {
    if (track_.empty()) {
        throw std::runtime_error("Track is empty, cannot select next waypoint");
    }
    if (player_.single_waypoint().waypoint_reached()) {
        assert_true(current_track_element_ != track_.end());
        ++current_track_element_;
    }
    if (current_track_element_ == track_.end()) {
        current_track_element_ = track_.begin();
    }
    player_.single_waypoint().set_waypoint(current_track_element_->position);
}

void PlaybackWaypoints::set_waypoints(
    const TransformationMatrix<double, 3>& inverse_geographic_mapping,
    const std::string& playback_filename)
{
    std::ifstream ifstr{playback_filename};
    if (ifstr.fail()) {
        throw std::runtime_error("Could not open file \"" + playback_filename + '"');
    }
    while (true) {
        TrackElement te = TrackElement::from_stream(ifstr, inverse_geographic_mapping);
        if (ifstr.fail()) {
            if (!ifstr.eof()) {
                throw std::runtime_error("Could not read from file \"" + playback_filename + '"');
            }
            break;
        }
        track_.push_back(te);
    }
}
