#pragma once
#include <list>
#include <string>

namespace Mlib {

template <class TData, size_t n>
class TransformationMatrix;
struct TrackElement;

class PlaybackWaypoints{
public:
    void drive();
    void set_waypoints(
        const TransformationMatrix<double, 3>& inverse_geographic_mapping,
        const std::string& playback_filename);
private:
    std::list<TrackElement> track_;
};

}
