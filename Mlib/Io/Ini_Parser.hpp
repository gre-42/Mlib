#pragma once
#include <Mlib/Strings/To_Number.hpp>
#include <map>
#include <string>

namespace Mlib {

class IniParser {
public:
    explicit IniParser(const std::string& filename);
    const std::string& get(const std::string& section, const std::string& key) const;
    template <class T>
    T get(const std::string& section, const std::string& key) const {
        return safe_stox<T>(get(section, key));
    }
    std::map<std::string, std::map<std::string, std::string>>::iterator begin();
    std::map<std::string, std::map<std::string, std::string>>::iterator end();
private:
    std::map<std::string, std::map<std::string, std::string>> content_;
};

}
