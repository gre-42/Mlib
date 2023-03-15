#include "Pacenote_Reader.hpp"
#include <Mlib/Json.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <fstream>

using json = nlohmann::json;

namespace Mlib {

struct PacenoteSection: Pacenote {
    size_t i0;
    size_t i1;
};

void from_json(const json& j, PacenoteDirection& dir) {
    dir = pacenote_direction_from_string(j.get<std::string>());
}

void from_json(const json& j, PacenoteSection& obj) {
    j.at("i0").get_to(obj.i0);
    j.at("i1").get_to(obj.i1);
    j.at("direction").get_to(obj.direction);
    j.at("gear").get_to(obj.gear);
}

}

using namespace Mlib;

PacenoteReader::PacenoteReader(
    const std::string& filename,
    size_t nlaps,
    size_t nread_ahead)
: nlaps_{nlaps},
  nread_ahead_{nread_ahead}
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
    pacenotes_ = j.at("pacenotes").get<std::vector<PacenoteSection>>();
    if ((nlaps > 1) && !pacenotes_.empty() && (pacenotes_.back().i1 < nframes_)) {
        THROW_OR_ABORT("Last pacenote too short");
    }
}

PacenoteReader::~PacenoteReader() = default;

const Pacenote* PacenoteReader::read(size_t frame_index, size_t lap_index) {
    if (nread_ahead_ >= nframes_) {
        return nullptr;
    }
    if (pacenotes_.empty()) {
        return nullptr;
    }
    size_t total_frame_index = frame_index + nread_ahead_;
    if ((total_frame_index >= nframes_) && (lap_index + 1 < nlaps_)) {
        total_frame_index -= nframes_;
    }
    if ((nlaps_ > 1) && (lap_index != 0)) {
        if ((total_frame_index + nframes_) < pacenotes_.back().i1) {
            return &pacenotes_.back();
        }
    }
    auto res = std::lower_bound(
        pacenotes_.begin(),
        pacenotes_.end(),
        total_frame_index,
        [](const PacenoteSection& pacenote_section, size_t frame_index){
            return pacenote_section.i1 < frame_index;
        });
    if (res == pacenotes_.end()) {
        return nullptr;
    }
    return &*res;
}
