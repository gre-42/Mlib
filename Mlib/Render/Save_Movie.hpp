#pragma once
#include <string>

namespace Mlib {

class SaveMovie {
public:
    void save(
        const std::string& file_prefix,
        const std::string& file_suffix,
        size_t width,
        size_t height,
        bool normalize = false);
private:
    size_t index_ = 0;
};

}
