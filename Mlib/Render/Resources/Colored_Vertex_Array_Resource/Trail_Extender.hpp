#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <chrono>
#include <map>
#include <optional>
#include <vector>

namespace Mlib {

template <class TData, size_t tshape0, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
struct ColoredVertex;
class TrailsInstance;
struct TrailSequence;

struct PreviousCenter {
    FixedArray<double, 3> position;
    std::chrono::steady_clock::time_point time;
};

class TrailExtender final: public ITrailExtender {
public:
    TrailExtender(
        TrailsInstance& trails_instance,
        const TrailSequence& trail_sequence,
        const UUVector<FixedArray<ColoredVertex<float>, 3>>& segment,
        double min_spawn_length,
        double max_spawn_length,
        float spawn_duration);
    virtual void append_location(
        const TransformationMatrix<float, double, 3>& location,
        TrailLocationType location_type) override;

private:
    TrailsInstance& trails_instance_;
    const TrailSequence& trail_sequence_;
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& segment_;
    double min_spawn_length_squared_;
    double max_spawn_length_squared_;
    float spawn_duration_;
    std::optional<PreviousCenter> previous_center_;
    std::map<OrderableFixedArray<float, 2>, FixedArray<double, 3>> previous_vertices_;
    std::map<OrderableFixedArray<float, 2>, FixedArray<double, 3>> current_vertices_;
};

}
