#pragma once
#include <map>
#include <string>

namespace Mlib {

class IniParser {
public:
    explicit IniParser(const std::string& filename);
    std::map<std::string, std::map<std::string, std::string>>::iterator begin();
    std::map<std::string, std::map<std::string, std::string>>::iterator end();
private:
    std::map<std::string, std::map<std::string, std::string>> content_;
};

}
