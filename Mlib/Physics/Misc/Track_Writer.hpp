#pragma once
#include <fstream>
#include <string>

namespace Mlib {

struct TrackElement;

class TrackWriter {
public:
    TrackWriter(const std::string& filename);
    void write(const TrackElement& e);
    void flush();
private:
    std::string filename_;
    std::ofstream ofstr_;
};

}
