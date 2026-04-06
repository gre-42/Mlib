#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <stb_cpp/stb_encode.hpp>
#include <stdexcept>

using namespace Mlib;

void stb_write_png(
    const std::string& filename,
    int width,
    int height,
    int nrChannels,
    const uint8_t* data)
{
    auto f = create_ofstream(filename, std::ios::binary);
    if (f->fail()) {
        throw std::runtime_error("Could not open \"" + filename + "\" for write");
    }
    auto tex = stb_encode_png(
        data,
        width,
        height,
        nrChannels);
    f->write((const char*)tex.data(), integral_cast<std::streamsize>(tex.size()));
    f->flush();
    if (f->fail()) {
        throw std::runtime_error("Could not write to \"" + filename + '"');
    }
}
