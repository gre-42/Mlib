#include "Ini_Parser.hpp"
#include <Mlib/Iterator/Enumerate.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/Trim.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

IniParser::IniParser(const std::filesystem::path& filename) {
    auto f = create_ifstream(filename);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename.string() + '"');
    }
    std::map<std::string, std::string>* section = &sections_.try_emplace("default", std::map<std::string, std::string>{}).first->second;
    std::string line;
    while (std::getline(*f, line)) {
        trim(line);
        if (line.empty()) {
            continue;
        }
        if ((line[0] == ';') || (line[0] == '#')) {
            continue;
        }
        if (line[0] == '[') {
            auto sec_name = line.substr(1, line.length() - 2);
            if (sec_name.ends_with("...")) {
                section = &lists_[sec_name].emplace_back();
            } else {
                section = &sections_[sec_name];
            }
            continue;
        }
        auto i = line.find('=');
        if (i == line.npos) {
            THROW_OR_ABORT("Could not parse line of INI file: \"" + line + '"');
        }
        if (!section->try_emplace(trim_copy(line.substr(0, i)), trim_copy(line.substr(i + 1))).second) {
            THROW_OR_ABORT("Found duplicate key in INI file: \"" + line + '"');
        }
    }
    if (!f->eof()) {
        THROW_OR_ABORT("Could not read from file \"" + filename.string() + '"');
    }
}

const std::string& IniParser::get(const std::string& section, const std::string& key) const
{
    auto sit = sections_.find(section);
    if (sit == sections_.end()) {
        THROW_OR_ABORT("Could not find section with name \"" + section + '"');
    }
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) {
        THROW_OR_ABORT("Could not find key with name \"" + key + '"');
    }
    return kit->second;
}

std::optional<std::string> IniParser::try_get(const std::string& section, const std::string& key) const {
    auto sit = sections_.find(section);
    if (sit == sections_.end()) {
        return std::nullopt;
    }
    auto kit = sit->second.find(key);
    if (kit == sit->second.end()) {
        return std::nullopt;
    }
    return kit->second;
}

const std::map<std::string, std::map<std::string, std::string>>& IniParser::sections() const {
    return sections_;
}

const std::map<std::string, std::list<std::map<std::string, std::string>>>& IniParser::lists() const {
    return lists_;
}

void IniParser::print(std::ostream& ostr) const {
    ostr << "Sections\n";
    for (const auto& [name, section] : sections_) {
        ostr << "  Section " << name << '\n';
        for (const auto& [k, v] : section) {
            ostr << "    " << k << " = " << v << '\n';
        }
    }
    ostr << "Lists\n";
    for (const auto& [name, list] : lists_) {
        ostr << "  List " << name << '\n';
        for (const auto& [i, map] : enumerate(list)) {
            ostr << "    " << i << '\n';
            for (const auto& [k, v] : map) {
                ostr << "      " << k << " = " << v << '\n';
            }
        }
    }
}
