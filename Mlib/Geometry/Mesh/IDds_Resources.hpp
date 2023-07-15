#pragma once
#include <iosfwd>
#include <string>

namespace Mlib {

class IDdsResources {
public:
    virtual void insert_dds_texture(const std::string& name, std::istream& istr) = 0;
};

}
