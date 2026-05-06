#include "Trail_Source.hpp"
#include <Mlib/Scene_Graph/Interfaces/ITrail_Extender.hpp>

using namespace Mlib;

TrailSource::TrailSource(
    std::unique_ptr<ITrailExtender> extender,
    const FixedArray<float, 3> position,
    float minimum_velocity)
    : extender{ std::move(extender) }
    , position{ position }
    , minimum_velocity{ minimum_velocity }
{}

TrailSource::~TrailSource() = default;
