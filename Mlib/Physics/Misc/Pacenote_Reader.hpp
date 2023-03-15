#pragma once
#include <Mlib/Physics/Misc/Pacenote.hpp>
#include <vector>

namespace Mlib {

struct PacenoteSection;

class PacenoteReader {
public:
    explicit PacenoteReader(
        const std::string& filename,
        size_t nlaps,
        size_t nread_ahead);
    ~PacenoteReader();
    const Pacenote* read(size_t frame_index, size_t lap_index);
private:
    size_t nlaps_;
    size_t nread_ahead_;
    size_t nframes_;
    std::vector<PacenoteSection> pacenotes_;
};

}
