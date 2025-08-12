#pragma once
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Audio/Audio_Source.hpp>
#include <Mlib/Math/Fixed_Point_Number.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Physics/Interfaces/IAdvance_Time.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <functional>
#include <list>
#include <memory>

namespace Mlib {

class EventEmitter;
class EventReceiverDeletionToken;
class AudioBuffer;

struct AudioSourceAndPosition {
    AudioSourceAndPosition(
        const AudioBuffer& buffer,
        PositionRequirement position_requirement,
        float alpha,
        const AudioSourceState<ScenePos>& position);
    AudioSource source;
    AudioSourceState<ScenePos> position;
};

class OneShotAudio final: public IAdvanceTime, public DanglingBaseClass {
    OneShotAudio(const OneShotAudio&) = delete;
    OneShotAudio &operator=(const OneShotAudio&) = delete;
public:
    explicit OneShotAudio(
        PositionRequirement position_requirement,
        std::function<bool()> paused,
        EventEmitter& paused_changed);
    ~OneShotAudio();
    void advance_time();
    virtual void advance_time(float dt, const StaticWorld& world) override;
    void play(
        const AudioBuffer& audio_buffer,
        const AudioSourceState<ScenePos>& position,
        float gain,
        float alpha = NAN);
    void stop();
private:
    PositionRequirement position_requirement_;
    std::list<AudioSourceAndPosition> sources_;
    mutable FastMutex mutex_;
    std::function<bool()> paused_;
    std::unique_ptr<EventReceiverDeletionToken> erdt_;
};

}
