#include "Ini_Parser.hpp"
#include <Mlib/Os/Os.hpp>
#include <Mlib/Strings/Trim.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

using namespace Mlib;

IniParser::IniParser(const std::string& filename) {
    auto f = create_ifstream(filename);
    if (f->fail()) {
        THROW_OR_ABORT("Could not open file \"" + filename + '"');
    }
    std::string section = "default";
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
            section = line.substr(1, line.length() - 2);
            continue;
        }
        auto i = line.find('=');
        if (i == line.npos) {
            THROW_OR_ABORT("Could not parse line of INI file: \"" + line + '"');
        }
        if (!content_[section].insert({line.substr(0, i), line.substr(i + 1)}).second) {
            THROW_OR_ABORT("Found duplicate key in INI file: \"" + line + '"');
        }
    }
    if (!f->eof()) {
        THROW_OR_ABORT("Could not read from file \"" + filename + '"');
    }
}

std::map<std::string, std::map<std::string, std::string>>::iterator IniParser::begin() {
    return content_.begin();
}

std::map<std::string, std::map<std::string, std::string>>::iterator IniParser::end() {
    return content_.end();
}
