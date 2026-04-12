#pragma once
#include <Mlib/Strings/String_View_To_Number.hpp>
#include <Mlib/Strings/Utf8_Path.hpp>
#include <iosfwd>
#include <list>
#include <map>
#include <optional>
#include <string>

namespace Mlib {

class IniParser {
public:
    explicit IniParser(const Utf8Path& filename);
    const std::string& get(const std::string& section, const std::string& key) const;
    template <class T>
    T get(const std::string& section, const std::string& key) const {
        return safe_sto<T>(get(section, key));
    }
    std::optional<std::string> try_get(const std::string& section, const std::string& key) const;
    template <class T>
    std::optional<T> try_get(const std::string& section, const std::string& key) const {
        auto s = try_get(section, key);
        if (!s.has_value()) {
            return std::nullopt;
        }
        return safe_sto<T>(*s);
    }
    const std::map<std::string, std::map<std::string, std::string>>& sections() const;
    const std::map<std::string, std::list<std::map<std::string, std::string>>>& lists() const;
    void print(std::ostream& ostr) const;
private:
    std::map<std::string, std::map<std::string, std::string>> sections_;
    std::map<std::string, std::list<std::map<std::string, std::string>>> lists_;
};

}
