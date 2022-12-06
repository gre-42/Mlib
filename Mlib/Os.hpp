#pragma once
#include <fstream>
#include <memory>

namespace Mlib {

std::unique_ptr<std::istream> create_ifstream(
    const std::string& filename,
    std::ios_base::openmode mode = std::ios_base::in);

bool file_exists(const std::string& filename);

}
