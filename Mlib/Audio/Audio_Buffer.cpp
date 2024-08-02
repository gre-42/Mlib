#include "Audio_Buffer.hpp"
#include <Mlib/Audio/Alut_Init_Without_Context.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>
#include <vector>

using namespace Mlib;

AudioBuffer::AudioBuffer(ALuint buffer)
: buffer_{buffer} {
    // AL_CHK(alGenBuffers((ALuint)1, &buffer_));
}

AudioBuffer::AudioBuffer(AudioBuffer&& other) noexcept {
    buffer_ = std::move(other.buffer_);
    other.buffer_.reset();
}

AudioBuffer::~AudioBuffer() {
    if (buffer_.has_value()) {
        AL_ABORT(alDeleteBuffers((ALuint)1, &(*buffer_)));
    }
}

// static ALenum to_al_format(short channels, short samples)
// {
//     bool stereo = (channels > 1);
// 
//     switch (samples) {
//         case 16:
//             if (stereo)
//                 return AL_FORMAT_STEREO16;
//             else
//                 return AL_FORMAT_MONO16;
//         case 8:
//             if (stereo)
//                 return AL_FORMAT_STEREO8;
//             else
//                 return AL_FORMAT_MONO8;
//         default:
//             THROW_OR_ABORT("Unsupported samples");
//     }
// }

AudioBuffer AudioBuffer::from_wave(const std::string& filename) {
    // WaveInfo* wave = WaveOpenFileForReading(filename.c_str());
    // if (wave == nullptr) {
    //     THROW_OR_ABORT("Failed to read wave file: \"" + filename + '"');
    // }

    // try {
    //     {
    //         int ret = WaveSeekFile(0, wave);
    //         if (ret != 0) {
    //             THROW_OR_ABORT("Failed to seek wave file: \"" + filename + '"');
    //         }
    //     }

    //     std::vector<char> bufferData(wave->dataSize);
    //     {
    //         int ret = WaveReadFile(bufferData.data(), wave->dataSize, wave);
    //         if (ret != (int)wave->dataSize) {
    //             THROW_OR_ABORT(
    //                 "Short read: " + std::to_string(ret) +
    //                 ", want: " + std::to_string(wave->dataSize) +
    //                 ", file: \"" + filename + '"');
    //         }
    //     }

    //     AL_CHK(alBufferData(
    //         buffer_, to_al_format(wave->channels, wave->bitsPerSample),
    //         bufferData.data(), wave->dataSize, wave->sampleRate));
    // } catch (...) {
    //     WaveCloseFile(wave);
    //     throw;
    // }
    // WaveCloseFile(wave);

    AlutInitWithoutContext alut_init_without_context;
    // ALuint buffer = alutCreateBufferFromFile(filename.c_str());
    auto data = read_file_bytes(filename);
    ALuint buffer = alutCreateBufferFromFileImage(data.data(), integral_cast<ALsizei>(data.size()));
    if (buffer == AL_NONE) {
        ALenum error = alutGetError();
        THROW_OR_ABORT("Could not load file \"" + filename + "\": " + alutGetErrorString(error) + ", code " + std::to_string(error));
    }
    return AudioBuffer{buffer};
}
