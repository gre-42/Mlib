#pragma once
#include <Mlib/Audio/Audio_Resources.hpp>
#include <Mlib/Audio/OpenAL_al.h>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <cstdint>
#include <memory>
#include <optional>
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
    static std::shared_ptr<AudioBuffer> from_wave(
        const Utf8Path& filename,
        std::optional<AudioLowpassInformation> lowpass = std::nullopt);
    static std::shared_ptr<AudioBuffer> from_mp3(
        const Utf8Path& filename,
        std::optional<AudioLowpassInformation> lowpass = std::nullopt);
    static std::shared_ptr<AudioBuffer> from_file(
        const Utf8Path& filename,
        std::optional<AudioLowpassInformation> lowpass = std::nullopt);
    uint32_t nchannels() const;

private:
    ALuint handle_;
};

}
