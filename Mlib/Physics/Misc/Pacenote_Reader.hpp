#pragma once
#include <Mlib/Physics/Misc/Pacenote.hpp>
#include <optional>
#include <vector>

namespace Mlib {

class PacenoteReader {
public:
    explicit PacenoteReader(
        const std::string& filename,
        size_t nlaps,
        double meters_read_ahead,
        double minimum_covered_meters);
    ~PacenoteReader();
    void read(
        double meters_to_start,
        size_t lap_index,
        std::vector<const Pacenote*>& pacenotes);
private:
    size_t nlaps_;
    double meters_read_ahead_;
    double minimum_covered_meters_;
    size_t nframes_;
    double length_in_meters_;
    std::vector<Pacenote> pacenotes_;
};

}
