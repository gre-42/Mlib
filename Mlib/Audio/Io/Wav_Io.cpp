#include "Wav_Io.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary_Reader.hpp>
#include <Mlib/Os/Io/Binary_Writer.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

void Mlib::write_wav_44_16_mono(const std::string& filename, const std::vector<int16_t>& data)
{
    auto wave_file = create_ofstream(filename, std::ios::binary);
    if (wave_file->fail()) {
        throw std::runtime_error("Could not open file for writing: \"" + filename + '"');
    }

    size_t fsize = data.size() * sizeof(data[0]);

    WavHeader header;
    header.chunk_size = integral_cast<uint32_t>(fsize + sizeof(header) - 8);
    header.subchunk2_size = integral_cast<uint32_t>(fsize);

    BinaryWriter binary_writer{*wave_file};
    binary_writer.write_binary(header, "wav header");
    binary_writer.write_iterable(data, "wav data");

    wave_file->flush();
    if (wave_file->fail()) {
        throw std::runtime_error("Could not write to wave file \"" + filename + '"');
    }
}

std::vector<int16_t> Mlib::read_wav_44_16_mono(const std::string& filename) {
    try {
        auto wave_file = create_ifstream(filename, std::ios::binary);
        if (wave_file->fail()) {
            throw std::runtime_error("Could not open file for reading");
        }
        BinaryReader binary_reader{*wave_file, IoVerbosity::SILENT};
        auto header = binary_reader.read_binary<WavHeader>("wav header");

        if (header.audio_format != 1) {
            throw std::runtime_error("No PCM format");
        }
        if (header.num_of_chan != 1) {
            throw std::runtime_error("Unexpected number of channels");
        }
        if (header.samples_per_sec != 44'100) {
            throw std::runtime_error("Only 44'100 Hz are supported");
        }
        if (header.bytes_per_sec != 44'100 * 2) {
            throw std::runtime_error("Unexpected bytes per second");
        }
        if (header.block_align != 2) {
            throw std::runtime_error("Only mono supported");
        }
        if (header.bits_per_sample != 16) {
            throw std::runtime_error("Only 16 bits per sample supported");
        }
        if (header.subchunk2_size > 100'000'000) {
            throw std::runtime_error("wav file too large");
        }
        if (header.subchunk2_size % 2 != 0) {
            throw std::runtime_error("Unsupported wav file format (only 16bit supported)");
        }
        std::vector<int16_t> result(header.subchunk2_size / 2);
        binary_reader.read_vector(result, "wav data", IoVerbosity::SILENT);
        return result;
    } catch (const std::runtime_error& e) {
        throw std::runtime_error("Error opening file \"" + filename + "\": " + e.what());
    }
}
