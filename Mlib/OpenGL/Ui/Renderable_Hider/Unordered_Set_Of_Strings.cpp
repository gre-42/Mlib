#include "Unordered_Set_Of_Strings.hpp"
#include <Mlib/Math/Sorted_Pointers.hpp>
#include <Mlib/Os/Os.hpp>

using namespace Mlib;

bool UnorderedSetOfStrings::contains(const VariableAndHash<std::string>& name) const {
    return strings_.contains(name);
}

void UnorderedSetOfStrings::try_insert(const VariableAndHash<std::string>& name, size_t max_entries) {
    auto capacity_ok = [&](){
        return strings_.size() < max_entries;
    };
    if (strings_.contains(name)) {
        return;
    } else if (!capacity_ok()) {
        lwarn(LogFlags::SUPPRESS_DUPLICATES) << "Too many renderables";
        return;
    }
    if (!capacity_ok()) {
        return;
    } else {
        strings_.insert(name);
    }
}

void UnorderedSetOfStrings::load_from_file(const Utf8Path& filename) {
    auto f = create_ifstream(filename);
    if (f->fail()) {
        lwarn() << "Could not open for loading: \"" << filename.string() << '"';
        return;
    }
    strings_.clear();
    std::string line;
    while (std::getline(*f, line)) {
        if (line.length() > 0 && line[line.length() - 1] == '\r') {
            line = line.substr(0, line.length() - 1);
        }
        if (line.length() == 0) {
            continue;
        }
        if (line[0] == '#') {
            continue;
        }
        strings_.insert(VariableAndHash{line});
    }
    if (!f->eof() && f->fail()) {
        throw std::runtime_error("Error reading from file \"" + filename.string() + '"');
    }
}

void UnorderedSetOfStrings::save_to_file(const Utf8Path& filename) const {
    auto f = create_ofstream(filename);
    if (f->fail()) {
        lwarn() << "Could not open for writing: \"" << filename.string() << '"';
        return;
    }
    auto sorted = sorted_pointers(strings_);
    for (const auto& pname : sorted) {
        const auto& s = **pname;
        f->write(s.data(), integral_cast<std::streamsize>(s.size()));
        f->put('\n');
    }
    f->flush();
    if (f->fail()) {
        lwarn() << "Could not write to file: \"" << filename.string() << '"';
        return;
    }
}
