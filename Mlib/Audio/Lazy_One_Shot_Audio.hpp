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
struct AudioFileInformation;
struct AudioSourceAndPosition;
enum class AudioPeriodicity;

class LazyOneShotAudio {
public:
    LazyOneShotAudio(
        AudioResources& resources,
        VariableAndHash<std::string> resource_name,
        float alpha = NAN);
    void preload();
    std::shared_ptr<AudioSourceAndPosition> play(
        OneShotAudio& one_shot_audio,
        AudioPeriodicity periodicity,
        const AudioSourceState<ScenePos>& position);
private:
    std::shared_ptr<AudioBuffer> buffer_;
    const AudioFileInformation* info_;
    float alpha_;
    AudioResources& resources_;
    VariableAndHash<std::string> resource_name_;
};

}
