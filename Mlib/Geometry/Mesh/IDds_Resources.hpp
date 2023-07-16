#pragma once
#include <string>
#include <vector>

namespace Mlib {

class IDdsResources {
public:
    virtual void insert_dds_texture(const std::string& name, std::vector<uint8_t>&& data) = 0;
};

}
