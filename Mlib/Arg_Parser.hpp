#pragma once
#include <algorithm>
#include <map>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

class CommandLineArgumentError: public std::runtime_error {
public:
    CommandLineArgumentError(const std::string& msg)
    : std::runtime_error(msg) {}
};

class ParsedArgs {
    std::string help;
    std::set<std::string> named_;
    std::map<std::string, std::string> named_values_;
    std::vector<std::string> unnamed_values_;
    friend class ArgParser;
public:
    bool has_named(const std::string& name) const {
        return named_.find(name) != named_.end();
    }
    bool has_named_value(const std::string& name) const {
        return named_values_.find(name) != named_values_.end();
    }
    std::string named_value(const std::string& name) const {
        if (!has_named_value(name)) {
            throw CommandLineArgumentError(help + "\n\nOption \"" + name + "\" is missing");
        }
        return named_values_.find(name)->second;
    }
    std::string named_value(const std::string& name, const std::string& deflt) const {
        if (!has_named_value(name)) {
            return deflt;
        }
        return named_values_.find(name)->second;
    }
    void assert_num_unamed(size_t num) const {
        if (unnamed_values_.size() != num) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
    }
    void assert_num_unamed_atleast(size_t num) const {
        if (unnamed_values_.size() < num) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
    }
    const std::string& unnamed_value(size_t index) const {
        if (index >= unnamed_values_.size()) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
        return unnamed_values_[index];
    }
    std::vector<std::string> unnamed_values(size_t begin = 0) const {
        if (begin > unnamed_values_.size()) {
            throw CommandLineArgumentError("Number of unnamed arguments not correct.\n" + help);
        }
        std::vector<std::string> result = unnamed_values_;
        for (size_t i = 0; i < begin; ++i) {
            result.erase(result.begin());
        }
        return result;
    }
};

class ArgParser {
    const std::string help;
    std::vector<std::string> options;
    std::vector<std::string> options_with_value;
    friend class OptIterator;
public:
    ArgParser(
        const std::string& help,
        std::vector<std::string> options,
        std::vector<std::string> options_with_value)
    : help(help),
      options(options),
      options_with_value(options_with_value) {}

    ParsedArgs parsed(int argc, char** argv) const {
        ParsedArgs result;
        result.help = help;
        bool eoopts = false;
        for (int i = 1; i < argc; ++i) {
            const std::string name = argv[i];
            if (!eoopts && name == "--") {
                eoopts = true;
            } else {
                if (!eoopts && (
                    std::find(
                        options.begin(),
                        options.end(),
                        name) != options.end())) {
                    if (!result.named_.insert(name).second) {
                        throw CommandLineArgumentError("Multiple values for " + name);
                    }
                } else if (!eoopts && (
                    std::find(
                        options_with_value.begin(),
                        options_with_value.end(),
                        name) != options_with_value.end())) {
                    if (i + 1 == argc) {
                        throw CommandLineArgumentError(help);
                    }
                    if (!result.named_values_.insert(std::make_pair(name, argv[i+1])).second) {
                        throw CommandLineArgumentError("Multiple values for " + name);
                    }
                    ++i;
                } else {
                    if (!eoopts && name.size() > 0 && name[0] == '-') {
                        throw CommandLineArgumentError(help);
                    }
                    result.unnamed_values_.push_back(argv[i]);
                }
            }
        }
        return result;
    }
};
