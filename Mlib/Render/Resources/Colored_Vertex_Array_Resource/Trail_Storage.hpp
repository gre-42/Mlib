#pragma once
#include <Mlib/Default_Uninitialized_Vector.hpp>
#include <Mlib/Render/Resources/Colored_Vertex_Array_Resource/Trail_Sequence.hpp>
#include <Mlib/Scene_Graph/Interfaces/ITrail_Storage.hpp>
#include <Mlib/Scene_Precision.hpp>
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
        const UUVector<FixedArray<ColoredVertex<float>, 3>>& segment,
        ScenePos min_spawn_length,
        ScenePos max_spawn_length,
        float spawn_duration);
    virtual std::unique_ptr<ITrailExtender> add_trail_extender() override;

private:
    TrailsInstance& trails_instance_;
    TrailSequence trail_sequence_;
    const UUVector<FixedArray<ColoredVertex<float>, 3>>& segment_;
    ScenePos min_spawn_length_;
    ScenePos max_spawn_length_;
    float spawn_duration_;
};

}
