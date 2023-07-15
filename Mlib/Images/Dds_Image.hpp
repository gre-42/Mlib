#pragma once
#include <Mlib/Array/Array.hpp>
#include <Mlib/Images/Rgba32.hpp>
#include <string>

namespace Mlib {

class DdsImage: public Array<Rgba32> {
public:
    DdsImage();
    ~DdsImage();

    static DdsImage load_from_file(const std::string& filename);
    static DdsImage load_from_stream(std::istream& istream);
private:
};

}
