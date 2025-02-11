#pragma once
#include <string>
#include <vector>

namespace Mlib {

struct KeyDescription {
    std::vector<std::string> required;
    std::string unique;
    std::string id;
    std::string section;
    std::string title;
};

}
