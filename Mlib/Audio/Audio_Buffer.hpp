#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <cstdint>
#include <memory>
#include <string>

namespace Mlib {

class AudioSource;

class AudioBuffer {
    friend AudioSource;
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer &operator=(const AudioBuffer&) = delete;

public:
    explicit AudioBuffer(ALuint handle);
    ~AudioBuffer();
    static std::shared_ptr<AudioBuffer> from_wave(const std::string& filename);
    uint32_t nchannels() const;

private:
    ALuint handle_;
};

}
