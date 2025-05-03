#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <stb_cpp/stb_encode.hpp>

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
        THROW_OR_ABORT("Could not open \"" + filename + "\" for write");
    }
    auto tex = stb_encode_png(
        data,
        width,
        height,
        nrChannels);
    f->write((const char*)tex.data(), integral_cast<std::streamsize>(tex.size()));
    f->flush();
    if (f->fail()) {
        THROW_OR_ABORT("Could not write to \"" + filename + '"');
    }
}
