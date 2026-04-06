#pragma once
#include <Mlib/Audio/OpenAL_al.h>
#include <cstdint>
#include <filesystem>
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
    static std::shared_ptr<AudioBuffer> from_wave(const std::filesystem::path& filename);
    static std::shared_ptr<AudioBuffer> from_mp3(const std::filesystem::path& filename);
    static std::shared_ptr<AudioBuffer> from_file(const std::filesystem::path& filename);
    uint32_t nchannels() const;

private:
    ALuint handle_;
};

}
