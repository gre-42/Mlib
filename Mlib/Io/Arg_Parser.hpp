#pragma once
#include <Mlib/Strings/Str.hpp>
#include <algorithm>
#include <filesystem>
#include <list>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

namespace Mlib {

class CommandLineArgumentError: public std::runtime_error {
public:
    CommandLineArgumentError(const std::string& msg)
    : std::runtime_error(msg) {}
};

class ParsedArgs {
    std::string help;
    std::set<std::string> named_;
    std::map<std::string, std::u8string> named_values_;
    std::map<std::string, std::list<std::u8string>> named_lists_;
    std::vector<std::u8string> unnamed_values_;
    friend class ArgParser;
public:
    bool has_named(const std::string& name) const {
        return named_.find(name) != named_.end();
    }
    bool has_named_value(const std::string& name) const {
        return named_values_.find(name) != named_values_.end();
    }
    bool has_named_list(const std::string& name) const {
        return named_lists_.find(name) != named_lists_.end();
    }
    std::u8string named_value(const std::string& name) const {
        if (!has_named_value(name)) {
            throw CommandLineArgumentError(help + "\n\nOption \"" + name + "\" is missing");
        }
        return named_values_.at(name);
    }
    std::string named_svalue(const std::string& name) const {
        if (!has_named_value(name)) {
            throw CommandLineArgumentError(help + "\n\nOption \"" + name + "\" is missing");
        }
        return U8::str(named_values_.at(name));
    }
    std::u8string named_value(const std::string& name, const std::u8string& deflt) const {
        if (!has_named_value(name)) {
            return deflt;
        }
        return named_values_.at(name);
    }
    std::u8string named_value(const std::string& name, const std::string& deflt) const {
        if (!has_named_value(name)) {
            return U8::u8str(deflt);
        }
        return named_values_.at(name);
    }
    std::string named_svalue(const std::string& name, const std::string& deflt) const {
        if (!has_named_value(name)) {
            return deflt;
        }
        return U8::str(named_values_.at(name));
    }
    const std::u8string* try_named_value(const std::string& name) const {
        if (!has_named_value(name)) {
            return nullptr;
        }
        return &named_values_.at(name);
    }
    const std::list<std::u8string>& named_list(const std::string& name) const {
        if (!has_named_list(name)) {
            throw CommandLineArgumentError(help + "\n\nOption \"" + name + "\" is missing");
        }
        return named_lists_.at(name);
    }
    void assert_num_unnamed(size_t num) const {
        if (unnamed_values_.size() != num) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
    }
    void assert_num_unnamed_atleast(size_t num) const {
        if (unnamed_values_.size() < num) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
    }
    const std::u8string& unnamed_value(size_t index) const {
        if (index >= unnamed_values_.size()) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
        return unnamed_values_[index];
    }
    std::vector<std::u8string> unnamed_values(size_t begin = 0) const {
        if (begin > unnamed_values_.size()) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
        const auto* data = unnamed_values_.data();
        return std::vector<std::u8string>(data + begin, data + unnamed_values_.size());
    }
};

class ArgParser {
    std::string help;
    std::set<std::string> options;
    std::set<std::string> options_with_value;
    std::set<std::string> options_with_list;
    friend class OptIterator;
public:
    ArgParser(
        std::string help,
        std::vector<std::string> options,
        std::vector<std::string> options_with_value,
        std::vector<std::string> options_with_list = {})
    : help(std::move(help)),
      options(options.begin(), options.end()),
      options_with_value(options_with_value.begin(), options_with_value.end()),
      options_with_list(options_with_list.begin(), options_with_list.end()) {}

    ParsedArgs parsed(int argc, char** argv) const {
        return parsed(argc, (const char**)argv);
    }
    ParsedArgs parsed(int argc, const char** argv) const {
        ParsedArgs result;
        result.help = help;
        bool eoopts = false;
        for (int i = 1; i < argc; ++i) {
            const std::string name = argv[i];
            if (!eoopts && name == "--") {
                eoopts = true;
            } else {
                if (!eoopts && options.contains(name)) {
                    if (!result.named_.insert(name).second) {
                        throw CommandLineArgumentError("Multiple values for " + name);
                    }
                } else if (!eoopts && options_with_value.contains(name)) {
                    if (i + 1 == argc) {
                        throw CommandLineArgumentError(help);
                    }
                    if (!result.named_values_.try_emplace(name, (char8_t*)argv[i+1]).second) {
                        throw CommandLineArgumentError("Multiple values for " + name);
                    }
                    ++i;
                } else if (!eoopts && options_with_list.contains(name)) {
                    ++i;
                    std::list<std::u8string> values;
                    for (; i < argc; ++i) {
                        std::u8string argi{(char8_t*)argv[i]};
                        if (!argi.empty() && (argi[0] == '-')) {
                            --i;
                            break;
                        }
                        values.push_back(argi);
                    }
                    if (!result.named_lists_.try_emplace(name, std::move(values)).second) {
                        throw CommandLineArgumentError("Multiple values for " + name);
                    }
                } else {
                    if (!eoopts && name.size() > 0 && name[0] == '-') {
                        throw CommandLineArgumentError(help);
                    }
                    result.unnamed_values_.emplace_back((char8_t*)argv[i]);
                }
            }
        }
        return result;
    }
};

}
