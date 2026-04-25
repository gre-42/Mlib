#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace Mlib {

// From: https://gist.github.com/csukuangfj/c1d1d769606260d436f8674c30662450
struct WavHeader {
    /* RIFF Chunk Descriptor */
    uint8_t riff[4] = { 'R', 'I', 'F', 'F' };           // RIFF Header Magic header
    uint32_t chunk_size;                                // RIFF Chunk Size
    uint8_t wave[4] = { 'W', 'A', 'V', 'E' };           // WAVE Header
    /* "fmt" sub-chunk */
    uint8_t fmt[4] = { 'f', 'm', 't', ' ' };            // FMT header
    uint32_t subchunk1_size = 16;                       // Size of the fmt chunk
    uint16_t audio_format = 1;                          // Audio format 1=PCM,6=mulaw,7=alaw, 257=IBM
                                                        // Mu-Law, 258=IBM A-Law, 259=ADPCM
    uint16_t num_of_chan = 1;                           // Number of channels 1=Mono 2=Sterio
    uint32_t samples_per_sec = 44'100;                  // Sampling Frequency in Hz
    uint32_t bytes_per_sec = 44'100 * 2;                // bytes per second
    uint16_t block_align = 2;                           // 2=16-bit mono, 4=16-bit stereo
    uint16_t bits_per_sample = 16;                      // Number of bits per sample
    /* "data" sub-chunk */
    uint8_t subchunk2id[4] = { 'd', 'a', 't', 'a' };    // "data"  string
    uint32_t subchunk2_size;                            // Sampled data length
};

static_assert(sizeof(WavHeader) == 44);

void write_wav_44_16_mono(const std::string& filename, const std::vector<int16_t>& data);
std::vector<int16_t> read_wav_44_16_mono(const std::string& filename);

}
