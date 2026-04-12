#ifdef WITHOUT_LAME

#include "Mp3_Io.hpp"
#include <stdexcept>

using namespace Mlib;

void Mlib::convert_to_mp3(
    const Utf8Path& source,
    const Utf8Path& destination,
    uint32_t bitrate)
{
    throw std::runtime_error("convert_to_mp3 not implemented on Android");
}

void Mlib::convert_to_mp3(
    std::basic_istream<int16_t>& source,
    std::ostream& destination,
    uint32_t bitrate)
{
    throw std::runtime_error("convert_to_mp3 not implemented on Android");
}

std::vector<std::byte> Mlib::convert_to_mp3(
    const std::vector<int16_t>& pcm,
    uint32_t bitrate)
{
    throw std::runtime_error("convert_to_mp3 not implemented on Android");
}

#else

#include "Mp3_Io.hpp"
#include <Mlib/Audio/Io/Wav_Io.hpp>
#include <Mlib/Io/Binary.hpp>
#include <Mlib/Memory/Destruction_Guard.hpp>
#include <Mlib/Os/Os.hpp>
#include <lame/lame.h>
#include <stdexcept>

using namespace Mlib;

void Mlib::convert_to_mp3(
    const Utf8Path& source,
    const Utf8Path& destination,
    uint32_t bitrate)
{
    // if (destination.extension() != ".mp3") {
    //     throw std::runtime_error("Extension is not mp3: \"" + destination.string() + '"');
    // }
    // auto ret = boost::process::system("ffmpeg", "-i", source.string(), "-ab", std::to_string(bitrate), destination.string());
    // if (ret != 0) {
    //     throw std::runtime_error("ffmpeg command failed");
    // }

    auto fo = create_ofstream(destination, std::ios::binary);
    if (fo->fail()) {
        throw std::runtime_error("Could not open file for write: \"" + destination.string() + '"');
    }
    auto wav = read_wav_44_16_mono(source);
    auto mp3 = convert_to_mp3(wav, 0);
    write_iterable(*fo, mp3, "MP3");
}

void Mlib::convert_to_mp3(
    std::basic_istream<int16_t>& source,
    std::ostream& destination,
    uint32_t bitrate)
{
    const int PCM_SIZE = 8192;
    const int MP3_SIZE = 8192;

    int16_t pcm_buffer[PCM_SIZE];
    uint8_t mp3_buffer[MP3_SIZE];

    lame_t lame = lame_init();
    if (lame == nullptr) {
        throw std::runtime_error("Could not initialize LAME");
    }
    DestructionGuard dg{[&](){
        lame_close(lame);
    }};
    lame_set_in_samplerate(lame, 44100);
    if (bitrate == 0) {
        lame_set_VBR(lame, vbr_default);
    } else {
        throw std::runtime_error("Only bitrate=0 is supported");
    }
    lame_set_num_channels(lame, 1);
    lame_set_mode(lame, MPEG_mode::MONO);
    lame_init_params(lame);

    while (true) {
        int read = integral_cast<int>(source.readsome(pcm_buffer, PCM_SIZE));
        if (source.fail()) {
            throw std::runtime_error("Could not read PCM stream");
        }
        int write;
        if (read == 0) {
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
        } else {
            write = lame_encode_buffer(lame, pcm_buffer, nullptr, read, mp3_buffer, MP3_SIZE);
        }
        destination.write((char*)mp3_buffer, write);
        if (read == 0) {
            break;
        }
    };
    destination.flush();
    if (destination.fail()) {
        throw std::runtime_error("Could not write to MP3 stream");
    }
}
    
std::vector<std::byte> Mlib::convert_to_mp3(
    const std::vector<int16_t>& pcm,
    uint32_t bitrate)
{
    std::stringstream ostr;
    std::basic_istringstream<int16_t> istr(
        std::basic_string<int16_t>(pcm.data(), pcm.size()));
    convert_to_mp3(istr, ostr, bitrate);
    ostr.seekg(0);
    return read_all_vector(ostr, "MP3", IoVerbosity::SILENT);
}

#endif
