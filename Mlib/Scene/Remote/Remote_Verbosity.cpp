#include "Remote_Verbosity.hpp"
#include <Mlib/Os/Io/Binary.hpp>

using namespace Mlib;

static IoVerbosity remote_verbosity = IoVerbosity::SILENT;

void Mlib::set_remote_io_verbosity(IoVerbosity verbosity) {
    remote_verbosity = verbosity;
}

IoVerbosity Mlib::get_remote_io_verbosity() {
    return remote_verbosity;
}
