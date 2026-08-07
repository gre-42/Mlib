#include "Ostream_Size_Logger.hpp"
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Os.hpp>
#include <istream>

using namespace Mlib;

OstreamSizeLogger::OstreamSizeLogger(std::ostream& ostr, std::string message)
    : ostr_{ostr}
    , message_{std::move(message)}
{
    ostr.seekp(0, std::ios::end);
    streampos_ = ostr.tellp();
}

OstreamSizeLogger::~OstreamSizeLogger() {
    ostr_.seekp(0, std::ios::end);
    auto len = integral_cast<size_t>(ostr_.tellp() - streampos_);
    linfo() << message_ << len;
}
