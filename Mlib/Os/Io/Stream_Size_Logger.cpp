#include "Stream_Size_Logger.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <istream>

using namespace Mlib;

StreamSizeLogger::StreamSizeLogger(std::istream& istr, std::string message)
    : istr_{istr}
    , message_{std::move(message)}
{
    istr.seekg(0, std::ios::end);
    streampos_ = istr.tellg();
}

StreamSizeLogger::~StreamSizeLogger() {
    istr_.seekg(0, std::ios::end);
    auto len = integral_cast<size_t>(istr_.tellg() - streampos_);
    linfo() << message_ << len;
}
