#pragma once
#include <Mlib/Strings/Utf8_Path.hpp>
#include <iosfwd>
#include <string>

namespace Mlib {

enum class IoVerbosity;
struct PssgModel;

PssgModel load_pssg(const Utf8Path& filename, IoVerbosity verbosity);
PssgModel load_pssg(
    std::istream& istr,
    const Utf8Path& filename,
    std::streamoff nbytes,
    IoVerbosity verbosity);

}
