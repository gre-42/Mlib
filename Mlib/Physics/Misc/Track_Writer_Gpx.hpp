#pragma once
#include <iosfwd>
#include <memory>
#include <string>

namespace Mlib {

template <typename TData, size_t... tshape>
class FixedArray;

class TrackWriterGpx {
public:
    TrackWriterGpx(const std::string& filename);
    ~TrackWriterGpx();
    void write(const FixedArray<double, 3>& position);
    void flush();
private:
    std::string filename_;
    std::unique_ptr<std::ostream> ofstr_;
};

}
