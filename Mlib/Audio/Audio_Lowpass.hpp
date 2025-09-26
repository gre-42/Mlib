#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <cstdint>
#include <memory>
#include <string>

namespace Mlib {

class AudioSource;
class AudioResources;
struct AudioLowpassInformation;

class AudioLowpass {
    friend AudioSource;
    friend AudioResources;
    AudioLowpass(const AudioLowpass&) = delete;
    AudioLowpass &operator=(const AudioLowpass&) = delete;
    
public:
    AudioLowpass();
    ~AudioLowpass();
    static std::shared_ptr<AudioLowpass> create(const AudioLowpassInformation& parameters);

private:
    ALuint handle_;
};

}
