#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <cmath>
#include <memory>
#include <string>

namespace Mlib {

class AudioBuffer;
class OneShotAudio;
class AudioResources;
template <class TPosition>
struct AudioSourceState;

class LazyOneShotAudio {
public:
    LazyOneShotAudio(
        AudioResources& resources,
        std::string resource_name,
        float alpha = NAN);
    void preload();
    void play(
        OneShotAudio& one_shot_audio,
        const AudioSourceState<ScenePos>& position);
private:
    std::shared_ptr<AudioBuffer> buffer_;
    float gain_;
    float alpha_;
    AudioResources& resources_;
    std::string resource_name_;
};

}
