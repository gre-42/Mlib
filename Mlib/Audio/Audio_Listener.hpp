#pragma once
#include <Mlib/Audio/Audio_Entity_State.hpp>
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Threads/Fast_Mutex.hpp>
#include <optional>

namespace Mlib {

class AudioListener {
    AudioListener() = delete;
    AudioListener(const AudioListener &) = delete;
    AudioListener &operator=(const AudioListener &) = delete;

public:
    static void set_transformation(const AudioListenerState& state);
    static std::optional<AudioSourceState<float>> get_relative_position(const AudioSourceState<ScenePos> &state);
    static void set_gain(float f);
    static void mute();
    static void unmute();

private:
    static FastMutex mutex_;
    static bool muted_;
    static float gain_;
    static std::optional<AudioListenerState> listener_inverse_state_;
};

}
