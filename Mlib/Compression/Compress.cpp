#include "Compress.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

#ifndef WITHOUT_ZLIB

static const uInt CHUNK = 16384;

CompressedBuf::CompressedBuf(const std::function<void(z_const std::string&)>& write)
    : std::stringbuf{std::ios::in | std::ios::out | std::ios::binary}
    , write_{ write }
{}

int CompressedBuf::sync() {
    {
        auto s = str();
        write_(s);
    }
    str("");
    return 0;
}

CompressedOStream::CompressedOStream(
    std::unique_ptr<std::ostream>&& ostr,
    std::streamsize in_length)
    : std::ostream{ &buf_ }
    , ostr_{ std::move(ostr) }
    , in_remaining_{ in_length }
    , write_{ [this](z_const std::string& s){
        if (s.empty()) {
            return;
        }
        auto predicted_remaining = in_remaining_ - integral_cast<std::streamsize>(s.length());
        if (predicted_remaining < 0) {
            throw std::runtime_error("Input stream longer than expected");
        }
        strm_.avail_in = integral_cast<uInt>(s.length());
        strm_.next_in = (unsigned char*)s.data();
        unsigned char out[CHUNK];
        do {
            strm_.avail_out = CHUNK;
            strm_.next_out = out;
            int ret = deflate(&strm_, (predicted_remaining == 0) ? Z_FINISH : Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR) {
                throw std::runtime_error("Could not deflate");
            }
            uInt have = CHUNK - strm_.avail_out;
            ostr_->write((char*)out, integral_cast<std::streamsize>(have));
        } while (strm_.avail_out == 0);
        in_remaining_ -= (s.length() - strm_.avail_in);
        if (strm_.avail_in != 0) {
            throw std::runtime_error("Could not read all input");
        }
        ostr_->flush();
    } }
    , buf_{ write_ }
{
    strm_.zalloc = Z_NULL;
    strm_.zfree = Z_NULL;
    strm_.opaque = Z_NULL;
    // From: https://stackoverflow.com/questions/1838699/how-can-i-decompress-a-gzip-stream-with-zlib
    if (int ret = deflateInit2(&strm_, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY); ret != Z_OK) {
        throw std::runtime_error("Could not initialize z_stream: " + std::to_string(ret));
    }
}

CompressedOStream::~CompressedOStream() {
    if (int ret = deflateEnd(&strm_); ret != Z_OK) {
        verbose_abort("deflateEnd failed: " + std::to_string(ret));
    }
}

#else

CompressedOStream::CompressedOStream(std::unique_ptr<std::ostream>&& ostr) {
    throw std::runtime_error("CompressedOStream requires ZLIB");
}

CompressedOStream::~CompressedOStream() = default;

#endif

void Mlib::compress_file(
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
    CompressedOStream co{std::move(of), integral_cast<std::streamsize>(in_size - std::streampos(0))};
    while (true) {
        auto c = i->get();
        if (i->eof()) {
            if (c != EOF) {
                throw std::runtime_error("EOF mismatch");
            }
            co.flush();
            if (co.fail()) {
                throw std::runtime_error("Could not write to file: \"" + destination.string() + '"');
            }
            break;
        }
        co.put((char)c);
        if (co.fail()) {
            throw std::runtime_error("Could not write to file: \"" + destination.string() + '"');
        }
    }
}

std::string Mlib::compressed_string(const std::string& source) {
    auto usstr = std::make_unique<std::stringstream>();
    auto& sstr = *usstr;
    CompressedOStream co{std::move(usstr), integral_cast<std::streamsize>(source.size())};
    for (char c : source) {
        co.put((char)c);
        if (co.fail()) {
            throw std::runtime_error("Could not write to stream");
        }
    }
    co.flush();
    if (co.fail()) {
        throw std::runtime_error("Could not write to stream");
    }
    sstr.seekg(0);
    return sstr.str();
}
