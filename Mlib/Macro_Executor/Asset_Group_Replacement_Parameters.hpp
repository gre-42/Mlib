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
    const ReplacementParameter& at(const std::string& name);
    inline auto begin() {
        return replacement_parameters_.begin();
    }
    inline auto end() {
        return replacement_parameters_.end();
    }
private:
    std::map<std::string, ReplacementParameter> replacement_parameters_;
    mutable std::shared_mutex mutex_;
};

}
