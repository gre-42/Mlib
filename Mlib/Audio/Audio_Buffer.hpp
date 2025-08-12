#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <optional>
#include <string>

namespace Mlib {

class AudioSource;

class AudioBuffer {
    friend AudioSource;
    AudioBuffer(const AudioBuffer&) = delete;
    AudioBuffer &operator=(const AudioBuffer&) = delete;

public:
    explicit AudioBuffer(ALuint buffer);
    AudioBuffer(AudioBuffer&& other) noexcept;
    ~AudioBuffer();
    static AudioBuffer from_wave(const std::string& filename);

private:
    std::optional<ALuint> buffer_;
};

}
