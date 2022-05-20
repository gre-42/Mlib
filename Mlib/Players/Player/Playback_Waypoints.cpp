#include "Playback_Waypoints.hpp"
#include <Mlib/Physics/Misc/Track_Element.hpp>
#include <fstream>

using namespace Mlib;

void PlaybackWaypoints::drive() {

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
