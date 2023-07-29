#pragma once
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

struct ReplacementParameter;
class MacroLineExecutor;
class JsonMacroArguments;

class AssetGroupReplacementParameters {
public:
    AssetGroupReplacementParameters();
    ~AssetGroupReplacementParameters();
    void insert(const std::string& filename, const MacroLineExecutor& mle);
    void merge(const std::string& id, const JsonMacroArguments& params);
    const ReplacementParameter& at(const std::string& id) const;
    std::map<std::string, ReplacementParameter>::iterator begin();
    std::map<std::string, ReplacementParameter>::iterator end();
private:
    std::map<std::string, ReplacementParameter> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
