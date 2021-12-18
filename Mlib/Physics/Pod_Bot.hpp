#pragma once
#include <string>

namespace Mlib {

class PodBot {
public:
    explicit PodBot(const std::string& name);
private:
    std::string name_;
};

}
