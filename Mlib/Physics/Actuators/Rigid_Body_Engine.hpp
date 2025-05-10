#pragma once
#include <Mlib/Physics/Actuators/Engine_Power.hpp>
#include <Mlib/Physics/Actuators/Engine_Power_Intent.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <set>
#include <vector>

namespace Mlib {

template <class TDir, class TPos, size_t n>
struct RotatingFrame;
template <typename TData, size_t... tshape>
class FixedArray;
class IEngineEventListener;
class EngineEventListeners;
struct EnginePowerDeltaIntent;
struct TirePowerIntent;
enum class VelocityClassification;
struct PhysicsTimeStep;
struct PhysicsPhase;

class RigidBodyEngine: public StatusWriter {
    friend std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);

    RigidBodyEngine(const RigidBodyEngine&) = delete;
    RigidBodyEngine& operator = (const RigidBodyEngine&) = delete;
public:
    explicit RigidBodyEngine(
        const std::optional<EnginePower>& engine_power,
        bool hand_brake_pulled,
        std::shared_ptr<IEngineEventListener> audio,
        std::shared_ptr<IEngineEventListener> exhaust);
    ~RigidBodyEngine();

    // StatusWriter
    virtual void write_status(std::ostream& ostr, StatusComponents status_components, const StaticWorld& world) const override;
    virtual float get_value(StatusComponents status_components) const override;
    virtual StatusWriter& child_status_writer(const std::vector<VariableAndHash<std::string>>& name) override;

    // Misc
    float surface_power() const;
    void set_surface_power(const EnginePowerIntent& engine_power_intent);
    TirePowerIntent consume_tire_power(
        size_t tire_id,
        const float* tire_w,
        const EnginePowerDeltaIntent& delta_intent,
        VelocityClassification velocity_classification);
    TirePowerIntent consume_rotor_power(
        size_t rotor_id,
        const float* rotor_w,
        const EnginePowerDeltaIntent& delta_intent);
    void reset_forces();
    void advance_time(
        const PhysicsTimeStep& dt,
        const PhysicsPhase& phase,
        const RotatingFrame<SceneDir, ScenePos, 3>& frame);
    float engine_w() const;

private:
    EnginePowerIntent engine_power_intent_;
    std::set<size_t> tires_consumed_;
    std::set<const float*> tires_w_;
    std::optional<EnginePower> engine_power_;
    size_t ntires_old_;
    bool hand_brake_pulled_;
    std::shared_ptr<EngineEventListeners> listeners_;
};

std::ostream& operator << (std::ostream& ostr, const TirePowerIntent& tire_power_intent);
std::ostream& operator << (std::ostream& ostr, const RigidBodyEngine& engine);

}
