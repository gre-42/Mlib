#pragma once
#include <Mlib/Array/Fixed_Array.hpp>
#include <Mlib/Math/Interp.hpp>
#include <Mlib/Math/Transformation/Position_Series.hpp>
#include <Mlib/Scene_Graph/Interfaces/IDynamic_Light.hpp>
#include <Mlib/Scene_Graph/Interpolation.hpp>
#include <chrono>
#include <functional>

namespace Mlib {

class DynamicLights;

struct AnimatedDynamicLightConfiguration {
    Interp<float, FixedArray<float, 3>> time_to_color = { {}, {} };
    Interp<double, float> squared_distance_to_intensity = { {}, {} };
};

class AnimatedDynamicLight: public IDynamicLight {
public:
    AnimatedDynamicLight(
        std::function<FixedArray<double, 3>()> get_position,
        std::chrono::steady_clock::time_point time,
        const AnimatedDynamicLightConfiguration& config,
        DynamicLights& container);
    virtual ~AnimatedDynamicLight() override;
    virtual void append_time(std::chrono::steady_clock::time_point time) override;
    virtual void set_time(std::chrono::steady_clock::time_point time) override;
    virtual FixedArray<float, 3> get_color(const FixedArray<double, 3>& target_position) const override;
    virtual bool animation_completed(std::chrono::steady_clock::time_point time) const override;

private:
    float elapsed(std::chrono::steady_clock::time_point time) const;

    std::chrono::steady_clock::time_point creation_time_;
    std::function<FixedArray<double, 3>()> get_position_;
    PositionSeries<double, 3, NINTERPOLATED> position_history_;
    const AnimatedDynamicLightConfiguration& config_;
    DynamicLights& container_;
    FixedArray<double, 3> position_;
    FixedArray<float, 3> color_;
};

}
