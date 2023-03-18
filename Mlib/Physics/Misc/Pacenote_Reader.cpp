#include "Pacenote_Reader.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Physics/Misc/Active_Pacenote.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Mlib {

struct PacenoteSection: Pacenote {
    size_t i0;
    size_t i1;
    double meters_to_start0;
    double meters_to_start1;
};

void from_json(const json& j, PacenoteDirection& dir) {
    dir = pacenote_direction_from_string(j.get<std::string>());
}

void from_json(const json& j, PacenoteSection& obj) {
    j.at("i0").get_to(obj.i0);
    j.at("i1").get_to(obj.i1);
    j.at("meters_to_start0").get_to(obj.meters_to_start0);
    j.at("meters_to_start1").get_to(obj.meters_to_start1);
    j.at("direction").get_to(obj.direction);
    j.at("gear").get_to(obj.gear);
}

}

using namespace Mlib;

PacenoteReader::PacenoteReader(
    const std::string& filename,
    size_t nlaps,
    double meters_read_ahead,
    double minimum_covered_meters)
: nlaps_{nlaps},
  meters_read_ahead_{meters_read_ahead},
  minimum_covered_meters_{minimum_covered_meters}
{
    if (nlaps == 0) {
        THROW_OR_ABORT("Number of laps must be at least 1");
    }
    auto ifstr = create_ifstream(filename);
    if (ifstr->fail()) {
        THROW_OR_ABORT("Could not open pacenote reader file \"" + filename + '"');
    }
    nlohmann::json j;
    *ifstr >> j;
    if (!ifstr->eof() && ifstr->fail()) {
        THROW_OR_ABORT("Error reading from file: \"" + filename + '"');
    }
    nframes_ = j.at("frames").get<size_t>();
    length_in_meters_ = j.at("length_in_meters").get<double>();
    pacenotes_ = j.at("pacenotes").get<std::vector<PacenoteSection>>();
    if ((nlaps > 1) && !pacenotes_.empty()) {
        if (pacenotes_.back().i1 < nframes_) {
            THROW_OR_ABORT("Last pacenote index too small");
        }
        if (pacenotes_.back().meters_to_start1 < length_in_meters_) {
            THROW_OR_ABORT("Last pacenote distance too small");
        }
    }
}

PacenoteReader::~PacenoteReader() = default;

void PacenoteReader::read(
    double meters_to_start,
    size_t lap_index,
    std::vector<ActivePacenote>& active_pacenotes)
{
    if (!active_pacenotes.empty()) {
        THROW_OR_ABORT("Active pacenotes not empty");
    }
    if (active_pacenotes.capacity() == 0) {
        THROW_OR_ABORT("Active pacenotes have capacity zero");
    }
    if (meters_read_ahead_ >= length_in_meters_) {
        return;
    }
    if (pacenotes_.empty()) {
        return;
    }
    double total_meters_to_start = meters_to_start + meters_read_ahead_;
    if (total_meters_to_start >= length_in_meters_) {
        if (lap_index + 1 < nlaps_) {
            total_meters_to_start -= length_in_meters_;
        } else {
            return;
        }
    }
    auto append_until_covered = [&](
        std::vector<PacenoteSection>::const_iterator it,
        double distance_in_meters,
        double covered_meters)
    {
        while ((covered_meters < minimum_covered_meters_) &&
                (active_pacenotes.size() < active_pacenotes.capacity()))
        {
            if ((++it) == pacenotes_.end()) {
                it = pacenotes_.begin();
            }
            distance_in_meters += (it->meters_to_start1 - it->meters_to_start0);
            covered_meters += (it->meters_to_start1 - it->meters_to_start0);
            active_pacenotes.push_back(ActivePacenote{
                .pacenote = &*it,
                .distance_in_meters = distance_in_meters,
                .length_in_meters = covered_meters});
        }

    };
    if ((nlaps_ > 1) && (lap_index != 0)) {
        if ((total_meters_to_start + length_in_meters_) < pacenotes_.back().meters_to_start0) {
            double distance_in_meters = pacenotes_.back().meters_to_start0 - (total_meters_to_start + length_in_meters_ - meters_read_ahead_);
            double covered_meters = pacenotes_.back().meters_to_start1 - pacenotes_.back().meters_to_start0;
            active_pacenotes.push_back(ActivePacenote{
                .pacenote = &pacenotes_.back(),
                .distance_in_meters = distance_in_meters,
                .length_in_meters = covered_meters});
            append_until_covered(
                pacenotes_.end(),
                distance_in_meters,
                covered_meters);
            return;
        }
    }
    auto res = std::lower_bound(
        pacenotes_.begin(),
        pacenotes_.end(),
        total_meters_to_start,
        [](const PacenoteSection& pacenote_section, double meters_to_start){
            return pacenote_section.meters_to_start0 < meters_to_start;
        });
    if (res == pacenotes_.end()) {
        return;
    }
    double distance_in_meters = res->meters_to_start0 - meters_to_start;
    double covered_meters = res->meters_to_start1 - res->meters_to_start0;
    active_pacenotes.push_back(ActivePacenote{
        .pacenote = &*res,
        .distance_in_meters = distance_in_meters,
        .length_in_meters = covered_meters});
    append_until_covered(
        res,
        distance_in_meters,
        covered_meters);
}
