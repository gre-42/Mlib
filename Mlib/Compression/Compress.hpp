#pragma once
#include <Mlib/Os/Utf8_Path.hpp>
#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#ifndef WITHOUT_ZLIB
#include <zlib.h>

namespace Mlib {

class CompressedBuf: public std::stringbuf {
    CompressedBuf(const CompressedBuf&) = delete;
    CompressedBuf& operator = (const CompressedBuf&) = delete;
public:
    explicit CompressedBuf(const std::function<void(z_const std::string&)>& write);
    virtual int sync() override;
private:
    const std::function<void(z_const std::string&)>& write_;
};

class CompressedOStream: public std::ostream {
    CompressedOStream(const CompressedOStream&) = delete;
    CompressedOStream& operator = (const CompressedOStream&) = delete;
public:
    CompressedOStream(
        std::unique_ptr<std::ostream>&& ostr,
        std::streamsize in_length);
    ~CompressedOStream() override;
private:
    z_stream strm_;
    std::unique_ptr<std::ostream> ostr_;
    std::streamsize in_remaining_;
    std::function<void(z_const std::string&)> write_;
    CompressedBuf buf_;
};

}

#else

namespace Mlib {
class CompressedOStream: public std::ostream {
    CompressedOStream(const CompressedOStream&) = delete;
    CompressedOStream& operator = (const CompressedOStream&) = delete;
public:
    explicit CompressedOStream(std::unique_ptr<std::ostream>&& ostr);
    ~CompressedOStream() override;
};

}

#endif

namespace Mlib {

void compress_file(
    const Utf8Path& source,
    const Utf8Path& destination);

std::string compressed_string(const std::string& source);

}
