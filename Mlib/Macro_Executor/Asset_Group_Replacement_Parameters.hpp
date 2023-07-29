#pragma once
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

struct ReplacementParameterAndFilename;
class MacroLineExecutor;
class JsonMacroArguments;

class AssetGroupReplacementParameters {
public:
    AssetGroupReplacementParameters();
    ~AssetGroupReplacementParameters();
    void insert(const std::string& filename, const MacroLineExecutor& mle);
    void merge_into_database(const std::string& id, const JsonMacroArguments& params);
    const ReplacementParameterAndFilename& at(const std::string& id) const;
    std::map<std::string, ReplacementParameterAndFilename>::iterator begin();
    std::map<std::string, ReplacementParameterAndFilename>::iterator end();
private:
    std::map<std::string, ReplacementParameterAndFilename> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
