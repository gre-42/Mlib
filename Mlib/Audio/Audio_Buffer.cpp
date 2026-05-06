#include "Audio_Buffer.hpp"
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/Io/Wav_Io.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Signal/Biquad_Filter.hpp>
#include <minimp3/minimp3_ex.h>
#include <mutex>
#include <stdexcept>
#include <vector>

using namespace Mlib;

AudioBuffer::AudioBuffer(ALuint handle)
    : handle_{ handle }
{
    // AL_CHK(alGenBuffers((ALuint)1, &handle_));
}

AudioBuffer::~AudioBuffer() {
    AL_ABORT(alDeleteBuffers((ALuint)1, &handle_));
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

std::shared_ptr<AudioBuffer> AudioBuffer::from_wave(
    const Utf8Path& filename,
    std::optional<AudioLowpassInformation> lowpass)
{
    // --------------------------
    // - Some 3rd party library -
    // --------------------------
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

    // --------
    // - ALUT -
    // --------
    // AlutInitWithoutContext alut_init_without_context;
    // // ALuint buffer = alutCreateBufferFromFile(filename.c_str());
    // auto data = read_file_bytes(filename);
    // ALuint buffer = alutCreateBufferFromFileImage(data.data(), integral_cast<ALsizei>(data.size()));
    // if (buffer == AL_NONE) {
    //     ALenum error = alutGetError();
    //     throw std::runtime_error("Could not load file \"" + filename.string() + "\": " + alutGetErrorString(error) + ", code " + std::to_string(error));
    // }
    // return std::make_shared<AudioBuffer>(buffer);

    std::vector<int16_t> pcm_data = read_wav_44_16_mono(filename);
    if (lowpass.has_value()) {
        BiquadFilter::process(std::span{pcm_data}, 5000.f, 44100.f, lowpass->gain, lowpass->gain_hf);
    }
    ALuint buffer;
    AL_CHK(alGenBuffers(1, &buffer));
    auto result = std::make_shared<AudioBuffer>(buffer);
    AL_CHK(alBufferData(buffer, AL_FORMAT_MONO16, pcm_data.data(), integral_cast<ALsizei>(pcm_data.size() * sizeof(int16_t)), 44'100));
    return result;
}

std::shared_ptr<AudioBuffer> AudioBuffer::from_mp3(
    const Utf8Path& filename,
    std::optional<AudioLowpassInformation> lowpass)
{
    auto file_data = read_file_bytes(filename);
    mp3dec_ex_t dec;
    if (mp3dec_ex_open_buf(&dec, (uint8_t*)file_data.data(), file_data.size(), MP3D_SEEK_TO_SAMPLE) != 0) {
        throw std::runtime_error("Could not open mp3: \"" + filename.string() + '"');
    }
    DestructionGuard dg{[&dec](){ mp3dec_ex_close(&dec); }};
    if (dec.info.channels != 1) {
        throw std::runtime_error("mp3 file does not have 1 channel: \"" + filename.string() + '"');
    }
    // Allocate memory for decoded samples
    std::vector<int16_t> pcm_data(dec.samples);
    size_t read_samples = mp3dec_ex_read(&dec, pcm_data.data(), dec.samples);
    if (lowpass.has_value()) {
        BiquadFilter::process(std::span{pcm_data}, 5000.f, integral_to_float<float>(dec.info.hz), lowpass->gain, lowpass->gain_hf);
    }
    ALuint buffer;
    AL_CHK(alGenBuffers(1, &buffer));
    auto result = std::make_shared<AudioBuffer>(buffer);
    AL_CHK(alBufferData(buffer, AL_FORMAT_MONO16, pcm_data.data(), integral_cast<ALsizei>(read_samples * sizeof(int16_t)), dec.info.hz));
    return result;
}

std::shared_ptr<AudioBuffer> AudioBuffer::from_file(
    const Utf8Path& filename,
    std::optional<AudioLowpassInformation> lowpass)
{
    if (filename.extension() == ".wav") {
        return from_wave(filename, lowpass);
    }
    if (filename.extension() == ".mp3") {
        return from_mp3(filename, lowpass);
    }
    throw std::runtime_error("Unknown audio file format: \"" + filename.string() + '"');
}

uint32_t AudioBuffer::nchannels() const {
    ALint value;
    AL_CHK(alGetBufferi(
        handle_,
        AL_CHANNELS,
        &value));
    return integral_cast<uint32_t>(value);
}
