#pragma once
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <set>
#include <vector>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;
class EngineEventListener;

enum class TirePowerIntentType {
    ACCELERATE,
    IDLE,
    BRAKE,
    BRAKE_OR_IDLE
};

struct TirePowerIntent {
    float power;
    float relaxation;
    TirePowerIntentType type;
};

enum class VelocityClassification {
    SLOW,
    FAST
};

class RigidBodyEngine: public StatusWriter {
    friend std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);

    RigidBodyEngine(const RigidBodyEngine&) = delete;
    RigidBodyEngine& operator = (const RigidBodyEngine&) = delete;
public:
    explicit RigidBodyEngine(
        const EnginePower& engine_power,
        bool hand_brake_pulled,
        const std::shared_ptr<EngineEventListener>& audio);
    ~RigidBodyEngine();

    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents status_components) const override;
    virtual float get_value(StatusComponents status_components) const override;
    virtual StatusWriter& child_status_writer(const std::vector<std::string>& name) override;

    // Misc
    float surface_power() const;
    void set_surface_power(const EnginePowerIntent& engine_power_intent);
    TirePowerIntent consume_abs_surface_power(
        size_t tire_id,
        const float* tire_w,
        VelocityClassification velocity_classification);
    void reset_forces();
    void advance_time(float dt, const FixedArray<double, 3>& position);
    float engine_w() const;

private:
    EnginePowerIntent engine_power_intent_;
    std::set<size_t> tires_consumed_;
    std::set<const float*> tires_w_;
    EnginePower engine_power_;
    size_t ntires_old_;
    bool hand_brake_pulled_;
    std::shared_ptr<EngineEventListener> audio_;
};

std::ostream& operator << (std::ostream& ostr, const TirePowerIntent& tire_power_intent);
std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);

}
