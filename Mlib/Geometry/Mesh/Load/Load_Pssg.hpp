#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

enum class IoVerbosity;
struct PssgModel;

PssgModel load_pssg(const std::string& filename, IoVerbosity verbosity);
PssgModel load_pssg(
    std::istream& istr,
    const std::string& filename,
    std::streamoff nbytes,
    IoVerbosity verbosity);

}
