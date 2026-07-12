#include "Audio_Buffer.hpp"
#include <Mlib/Audio/Audio_Device.hpp>
#include <Mlib/Audio/CHK.hpp>
#include <Mlib/Audio/Io/Wav_Io.hpp>
#include <Mlib/Images/Normalize_Integral.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Memory/Integral_To_Float.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Signal/Biquad_Filter.hpp>
#include <Mlib/Signal/Resample_1D.hpp>
#include <Mlib/Stats/Mean.hpp>
#include <minimp3/minimp3_ex.h>
#include <mutex>
#include <stdexcept>
#include <vector>
#ifdef __EMSCRIPTEN__
#include <AL/alext.h>
#endif

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
    if (dec.samples > 50'000'000) {
        throw std::runtime_error("mp3 file requires more than 100MB after decoding: \"" + filename.string() + '"');
    }
    Array<int16_t> pcm_data(ArrayShape{integral_cast<size_t>(dec.samples)});
    {
        size_t read_samples = mp3dec_ex_read(&dec, pcm_data.flat_begin(), dec.samples);
        if (read_samples > dec.samples) {
            throw std::runtime_error("mp3 file has more samples than expected after decoding: \"" + filename.string() + '"');
        }
        pcm_data.reshape(read_samples);
    }
    if (lowpass.has_value()) {
        BiquadFilter::process(std::span{pcm_data.flat_begin(), pcm_data.length()}, 5000.f, integral_to_float<float>(dec.info.hz), lowpass->gain, lowpass->gain_hf);
    }
    #ifdef __EMSCRIPTEN__
    {
        if ((dec.info.hz < 8'000) || (dec.info.hz > 48'000)) {
            throw std::runtime_error("Unexpected sampling rate: \"" + filename.string() + '"');
        }
        auto device_frequency = AudioDevice::get_frequency();
        if ((device_frequency < 8'000) || (device_frequency > 48'000)) {
            throw std::runtime_error("Unexpected device frequency: \"" + filename.string() + '"');
        }
        auto pcm_data_float = clipped(
            resample_1d(
                normalized_integral<float>(pcm_data, -1.f, 1.f),
                1.f / integral_to_float<float>(dec.info.hz),
                1.f / integral_to_float<float>(device_frequency)),
            -1.f, 1.f);
        if (!all(isfinite(pcm_data_float))) {
            throw std::runtime_error("Audio data contains NaN of infinity: \""  + filename.string() + '"');
        }
        if (!all(abs(pcm_data_float) <= 1.f)) {
            throw std::runtime_error("Audio data out of bounds: \""  + filename.string() + '"');
        }
        if (pcm_data_float.length() < 1'000) {
            throw std::runtime_error("Audio buffer has less than 1k samples: \""  + filename.string() + '"');
        }
        if (auto m = mean(pcm_data_float); m > 1e-1f) {
            throw std::runtime_error((std::stringstream() << "Audio data has nonzero mean (" <<
                m << "): \"" << filename.string() << '"').str());
        }
        {
            const auto SILENCE_THRESHOLD_DBFS = -60.f;
            const auto LOUDNESS_THRESHOLD_DBFS = -4.f;
            const auto MIN_MEAN_SQUARE = std::pow(10.f, SILENCE_THRESHOLD_DBFS / 10.f);
            const auto MAX_MEAN_SQUARE = std::pow(10.f, LOUDNESS_THRESHOLD_DBFS / 10.f);

            auto mean_square = mean(squared(pcm_data_float));
            if (mean_square < MIN_MEAN_SQUARE) {
                throw std::runtime_error("Audio is too silent: \""  + filename.string() + '"');
            }
            if (mean_square > MAX_MEAN_SQUARE) {
                auto dBFS = 10.f * std::log10(mean_square);
                throw std::runtime_error((std::stringstream() << "Audio is too loud (dBFS=" <<
                dBFS << "): \"" << filename.string() << '"').str());
            }
        }
        ALuint buffer;
        AL_CHK(alGenBuffers(1, &buffer));
        auto result = std::make_shared<AudioBuffer>(buffer);
        AL_CHK(alBufferData(buffer, AL_FORMAT_MONO_FLOAT32, pcm_data_float.flat_begin(),
            integral_cast<ALsizei>(pcm_data_float.length() * sizeof(float)), integral_cast<int>(device_frequency)));
        // linfo() << filename.string() <<
        //     " | Samples: " << pcm_data_float.length() <<
        //     " | HZ: " << device_frequency <<
        //     " | Total Bytes: " << (pcm_data_float.length() * sizeof(float));
        return result;
    }
    #else
    {
        ALuint buffer;
        AL_CHK(alGenBuffers(1, &buffer));
        auto result = std::make_shared<AudioBuffer>(buffer);
        AL_CHK(alBufferData(buffer, AL_FORMAT_MONO16, pcm_data.flat_begin(), integral_cast<ALsizei>(pcm_data.length() * sizeof(int16_t)), dec.info.hz));
        return result;
    }
    #endif
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
