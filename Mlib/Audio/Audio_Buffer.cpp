#include "Audio_Buffer.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/Alut_Init.hpp>
#include <vector>

using namespace Mlib;

AudioBuffer::AudioBuffer() {
    AL_CHK(alGenBuffers((ALuint)1, &buffer_));
}

AudioBuffer::~AudioBuffer() {
    AL_WARN(alDeleteBuffers((ALuint)1, &buffer_));
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
//             throw std::runtime_error("Unsupported samples");
//     }
// }

void AudioBuffer::load_wave(const std::string& filename) {
    // WaveInfo* wave = WaveOpenFileForReading(filename.c_str());
    // if (wave == nullptr) {
    //     throw std::runtime_error("Failed to read wave file: \"" + filename + '"');
    // }

    // try {
    //     {
    //         int ret = WaveSeekFile(0, wave);
    //         if (ret != 0) {
    //             throw std::runtime_error("Failed to seek wave file: \"" + filename + '"');
    //         }
    //     }

    //     std::vector<char> bufferData(wave->dataSize);
    //     {
    //         int ret = WaveReadFile(bufferData.data(), wave->dataSize, wave);
    //         if (ret != (int)wave->dataSize) {
    //             throw std::runtime_error(
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

    AlutInit alut_init;
    buffer_ = alutCreateBufferFromFile(filename.c_str());
    if (buffer_ == AL_NONE) {
        ALenum error = alutGetError();
        throw std::runtime_error("Fould not load file \"" + filename + "\": " + alutGetErrorString(error) + ", code " + std::to_string(error));
    }
}
