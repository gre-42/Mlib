#pragma once

namespace Mlib {

enum class IoVerbosity;

void set_remote_io_verbosity(IoVerbosity verbosity);
IoVerbosity get_remote_io_verbosity();

}
