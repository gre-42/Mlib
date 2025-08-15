#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Variable_And_Hash.hpp>
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
        VariableAndHash<std::string> resource_name,
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
    VariableAndHash<std::string> resource_name_;
};

}
