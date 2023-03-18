#pragma once
#include <Mlib/Physics/Misc/Pacenote.hpp>
#include <optional>
#include <vector>

namespace Mlib {

struct PacenoteSection;
struct ActivePacenote;

class PacenoteReader {
public:
    explicit PacenoteReader(
        const std::string& filename,
        size_t nlaps,
        double meters_read_ahead);
    ~PacenoteReader();
    std::optional<ActivePacenote> read(double meters_to_start, size_t lap_index);
private:
    size_t nlaps_;
    double meters_read_ahead_;
    size_t nframes_;
    double length_in_meters_;
    std::vector<PacenoteSection> pacenotes_;
};

}
