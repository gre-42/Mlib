#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>
#include <map>
#include <optional>
#include <vector>

namespace Mlib {

template <class TData, size_t... tshape>
class OrderableFixedArray;
template <class TPos>
struct ColoredVertex;
class TrailsInstance;
struct TrailSequence;

struct PreviousCenter {
    FixedArray<double, 3> position;
    double time;
};

class TrailExtender final: public ITrailExtender {
public:
    TrailExtender(
        TrailsInstance& trails_instance,
        const TrailSequence& trail_sequence,
        const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment,
        double minimum_length);
    virtual void append_location(const TransformationMatrix<float, double, 3>& location) override;

private:
    TrailsInstance& trails_instance_;
    const TrailSequence& trail_sequence_;
    const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment_;
    double minimum_length_;
    std::optional<PreviousCenter> previous_center_;
    std::map<OrderableFixedArray<float, 2>, FixedArray<double, 3>> previous_vertices_;
    std::map<OrderableFixedArray<float, 2>, FixedArray<double, 3>> current_vertices_;
};

}
