#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <memory>
#include <string>

namespace Mlib {

class AudioSource;
class AudioResources;
struct AudioEqualizerInformation;

class AudioEqualizer {
    friend AudioSource;
    friend AudioResources;
    AudioEqualizer(const AudioEqualizer&) = delete;
    AudioEqualizer &operator=(const AudioEqualizer&) = delete;

public:
    AudioEqualizer();
    ~AudioEqualizer();
    static std::shared_ptr<AudioEqualizer> create(const AudioEqualizerInformation& parameters);

private:
    ALuint handle_;
};

}
