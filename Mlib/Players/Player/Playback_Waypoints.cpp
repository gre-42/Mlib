#include "Playback_Waypoints.hpp"
#include <Mlib/Assert.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <Mlib/Physics/Units.hpp>
#include <Mlib/Players/Advance_Times/Player.hpp>
#include <Mlib/Scene_Graph/Way_Point_Location.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using namespace Mlib;

PlaybackWaypoints::PlaybackWaypoints(Player& player)
: player_{player},
  current_track_element_{track_.end()},
  speedup_{NAN}
{}

PlaybackWaypoints::~PlaybackWaypoints() = default;

bool PlaybackWaypoints::has_waypoints() const {
    return !track_.empty();
}

void PlaybackWaypoints::select_next_waypoint() {
    if (track_.empty()) {
        THROW_OR_ABORT("Track is empty, cannot select next waypoint");
    }
    if (!player_.single_waypoint().has_waypoint()) {
        current_track_element_ = track_.begin();
    } else if (player_.single_waypoint().waypoint_reached()) {
        if (current_track_element_ == track_.end()) {
            verbose_abort("Invalid track element");
        }
        auto old_element = current_track_element_;
        ++current_track_element_;
        if (current_track_element_ == track_.end()) {
            current_track_element_ = track_.begin();
        } else {
            float ds = (float)std::sqrt(sum(squared(
                current_track_element_->transformation().position() -
                old_element->transformation().position()))) * meters;
            float dt = (current_track_element_->elapsed_seconds - old_element->elapsed_seconds) * seconds;
            player_.single_waypoint().set_target_velocity(speedup_ * ds / dt);
        }
    }
    player_.single_waypoint().set_waypoint({
        current_track_element_->transformation().position().casted<CompressedScenePos>(),
        WayPointLocation::UNKNOWN });
}

void PlaybackWaypoints::set_waypoints(
    const TransformationMatrix<double, double, 3>& inverse_geographic_mapping,
    const std::string& playback_filename,
    float speedup)
{
    auto ifstr = create_ifstream(playback_filename);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not open waypoint file \"" + playback_filename + '"');
    }
    while (true) {
        TrackElement te = TrackElement::from_stream(*ifstr, inverse_geographic_mapping, 1);
        if (ifstr->fail()) {
            if (!ifstr->eof()) {
                THROW_OR_ABORT("Could not read from file \"" + playback_filename + '"');
            }
            break;
        }
        track_.push_back(te);
    }
    speedup_ = speedup;
}
