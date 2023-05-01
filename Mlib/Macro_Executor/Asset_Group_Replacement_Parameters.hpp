#pragma once
#include <map>
#include <shared_mutex>
#include <string>

namespace Mlib {

struct ReplacementParameter;
class MacroLineExecutor;

class AssetGroupReplacementParameters {
public:
    AssetGroupReplacementParameters();
    ~AssetGroupReplacementParameters();
    void insert(const std::string& filename, const MacroLineExecutor& mle);
    const ReplacementParameter& at(const std::string& id);
    std::map<std::string, ReplacementParameter>::iterator begin();
    std::map<std::string, ReplacementParameter>::iterator end();
private:
    std::map<std::string, ReplacementParameter> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
