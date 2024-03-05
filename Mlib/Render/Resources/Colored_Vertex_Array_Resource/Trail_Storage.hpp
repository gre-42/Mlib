#pragma once
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <vector>

namespace Mlib {

class TrailsInstance;
template <typename TData, size_t... tshape>
class FixedArray;
template <class TPos>
struct ColoredVertex;

class TrailStorage final: public ITrailStorage {
public:
    TrailStorage(
        TrailsInstance& trails_instance,
        TrailSequence trail_sequence,
        const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment,
        double minimum_length);
    virtual std::unique_ptr<ITrailExtender> add_trail_extender() override;

private:
    TrailsInstance& trails_instance_;
    TrailSequence trail_sequence_;
    const std::vector<FixedArray<ColoredVertex<float>, 3>>& segment_;
    double minimum_length_;
};

}
