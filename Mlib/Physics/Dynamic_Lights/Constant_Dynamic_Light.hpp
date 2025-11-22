#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation/Position_Series.hpp>
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Light.hpp>
#include <Mlib/Scene_Graph/Interpolation.hpp>
#include <chrono>
#include <functional>

namespace Mlib {

class DynamicLights;

struct ConstantDynamicLightConfiguration {
    FixedArray<float, 3> color = uninitialized;
    Interp<ScenePos, float> squared_distance_to_intensity = { {}, {} };
};

class ConstantDynamicLight: public IDynamicLight {
public:
    ConstantDynamicLight(
        std::function<FixedArray<ScenePos, 3>()> get_position,
        std::chrono::steady_clock::time_point time,
        const ConstantDynamicLightConfiguration& config,
        DynamicLights& container);
    virtual ~ConstantDynamicLight() override;
    virtual void append_time(std::chrono::steady_clock::time_point time) override;
    virtual void set_time(std::chrono::steady_clock::time_point time) override;
    virtual FixedArray<float, 3> get_color(const FixedArray<ScenePos, 3>& target_position) const override;
    virtual bool animation_completed(std::chrono::steady_clock::time_point time) const override;

private:
    std::function<FixedArray<ScenePos, 3>()> get_position_;
    FixedArray<ScenePos, 3> position_;
    PositionSeries<ScenePos, 3, NINTERPOLATED> position_history_;
    const ConstantDynamicLightConfiguration& config_;
    DynamicLights& container_;
};

}
