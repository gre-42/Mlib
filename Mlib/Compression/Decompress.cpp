#include "Decompress.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Binary.hpp>
#include <Mlib/Os/Os.hpp>
#include <functional>
#include <iostream>
#include <list>
#include <vector>
#ifndef WITHOUT_ZLIB
#include <zlib.h>
#endif

using namespace Mlib;

#ifdef WITHOUT_ZLIB

std::istringstream Mlib::uncompress_stream(
    std::istream& istr,
    const std::string& filename,
    std::streamoff nbytes,
    size_t chunk_size)
{
    throw std::runtime_error("Mlib::uncompress_stream requires ZLIB");
}

#else

int uncompress2_patched(Bytef *dest, uLongf destLen,
                        const Bytef *source, uLong *sourceLen,
                        const std::function<bool(uLongf chunkLen)>& notifyRead)
{
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len;

    if (destLen == 0) {
        return Z_BUF_ERROR;
    }

    len = *sourceLen;

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)nullptr;
    stream.zfree = (free_func)nullptr;
    stream.opaque = (voidpf)nullptr;

    // From: https://stackoverflow.com/questions/1838699/how-can-i-decompress-a-gzip-stream-with-zlib
    err = inflateInit2(&stream, 16 + MAX_WBITS);
    if (err != Z_OK) return err;

    stream.total_out = 0;
    while (true) {
        stream.next_out = dest;
        stream.avail_out = destLen > (uLong)max ? max : (uInt)destLen;

        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
        }
        auto before = stream.total_out;
        err = inflate(&stream, Z_NO_FLUSH);
        if ((err == Z_OK) || (err == Z_STREAM_END)) {
            if (!notifyRead(stream.total_out - before)) {
                return Z_BUF_ERROR;
            }
        }
        if (err != Z_OK) {
            break;
        }
    }

    *sourceLen -= len + stream.avail_in;

    inflateEnd(&stream);
    return err == Z_STREAM_END ? Z_OK :
           err == Z_NEED_DICT ? Z_DATA_ERROR :
           err == Z_BUF_ERROR ? Z_DATA_ERROR :
           err;
}

std::istringstream Mlib::uncompress_stream(
    std::istream& istr,
    const std::string& filename,
    std::streamoff nbytes,
    size_t chunk_size)
{
    auto begin = istr.tellg();
    std::streampos end;
    if (nbytes == std::numeric_limits<std::streamoff>::max()) {
        istr.seekg(0, std::ios::end);
        end = istr.tellg();
        istr.seekg(begin);
    } else {
        end = begin + nbytes;
    }
    std::vector<char> compressed(integral_cast<size_t>(end - begin));
    read_vector(istr, compressed, "compressed data", IoVerbosity::SILENT);

    auto clength = integral_cast<uLongf>(compressed.size());
    std::string uncompressed_chunk(chunk_size, '?');
    std::list<std::string> uncompressed_chunks;
    auto notify_read = [&](uLongf chunkSize){
        if (uncompressed_chunks.size() + chunkSize > 1'000'000'000) {
            return false;
        }
        uncompressed_chunks.push_back(uncompressed_chunk.substr(0, chunkSize));
        return true;
    };
    if (int ret = uncompress2_patched(
            (Bytef*)uncompressed_chunk.data(),
            integral_cast<uLongf>(uncompressed_chunk.size()),
            (Bytef*)compressed.data(),
            &clength,
            notify_read);
        ret != Z_OK)
    {
        istr.seekg(begin);
        switch (ret) {
        case Z_MEM_ERROR:
            throw std::runtime_error("Not enough memory for decompression: \"" + filename + '"');
        case Z_BUF_ERROR:
            throw std::runtime_error("Not enough room in the output buffer: \"" + filename + '"');
        case Z_DATA_ERROR:
            throw std::runtime_error("Input data corrupted or incomplete: \"" + filename + '"');
        }
        verbose_abort("Unknown return code in uncompress: " + std::to_string(ret) + ", \"" + filename + '"');
    }
    istr.seekg(begin + integral_cast<std::streamoff>(clength));
    compressed.clear();
    size_t total_size = 0;
    for (const auto& c : uncompressed_chunks) {
        total_size += c.size();
    }
    std::string uncompressed_string;
    uncompressed_string.reserve(total_size);
    for (const auto& c : uncompressed_chunks) {
        uncompressed_string += c;
    }
    return std::istringstream{ uncompressed_string, std::ios::binary | std::ios::in };
}
#endif

void Mlib::decompress_file(
    const Utf8Path& source,
    const Utf8Path& destination)
{
    auto i = create_ifstream(source, std::ios::binary);
    if (i->fail()) {
        throw std::runtime_error("Could not open file for read: \"" + source.string() + '"');
    }
    auto of = create_ofstream(destination, std::ios::binary);
    if (of->fail()) {
        throw std::runtime_error("Could not open file for write: \"" + source.string() + '"');
    }
    i->seekg(0, std::ios::end);
    auto in_size = i->tellg();
    i->seekg(0);
    auto uc = uncompress_stream(*i, source, in_size);
    while (true) {
        auto c = uc.get();
        if (uc.eof()) {
            if (c != EOF) {
                throw std::runtime_error("EOF mismatch");
            }
            of->flush();
            if (of->fail()) {
                throw std::runtime_error("Could not write to file: \"" + destination.string() + '"');
            }
            break;
        }
        of->put((char)c);
        if (of->fail()) {
            throw std::runtime_error("Could not write to file: \"" + destination.string() + '"');
        }
    }
}
