#pragma once

#ifdef __clang__

namespace Mlib {

class SourceLocation {
public:
    inline SourceLocation(const char* file_name, int line)
    : file_name_{file_name},
      line_{line}
    {}
    inline const char* file_name() const {
        return file_name_;
    }
    inline int line() const {
        return line_;
    }
private:
    const char* file_name_;
    int line_;
};
#define CURRENT_SOURCE_LOCATION SourceLocation{__FILE__, __LINE__}

}

#else

#include <source_location>

namespace Mlib {

using SourceLocation = std::source_location;
#define CURRENT_SOURCE_LOCATION std::source_location::current()

}

#endif
