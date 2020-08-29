#pragma once
#include <fstream>
#include <list>
#include <string>

namespace Mlib {

std::list<std::string> read_lines_from_file(const std::string& filename) {
    std::list<std::string> result;
    std::ifstream ifs(filename);
    while(!ifs.eof()) {
        std::string line;
        std::getline(ifs, line);
        if (ifs.fail()) {
            throw std::runtime_error("Could not read from " + filename);
        }
        result.push_back(line);
    }
    return result;
}

}
