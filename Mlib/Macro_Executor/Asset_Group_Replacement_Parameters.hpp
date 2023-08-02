#pragma once
#include <list>
#include <map>
#include <memory>
#include <shared_mutex>
#include <string>

namespace Mlib {

struct ReplacementParameterAndFilename;
class MacroLineExecutor;
class JsonMacroArguments;
class IAssetLoader;

class AssetGroupReplacementParameters {
public:
    AssetGroupReplacementParameters();
    ~AssetGroupReplacementParameters();
    void insert(const std::string& filename, const MacroLineExecutor& mle);
    void insert(const ReplacementParameterAndFilename& rp);
    void merge_into_database(const std::string& id, const JsonMacroArguments& params);
    const ReplacementParameterAndFilename& at(const std::string& id) const;
    std::map<std::string, ReplacementParameterAndFilename>::iterator begin();
    std::map<std::string, ReplacementParameterAndFilename>::iterator end();
    void add_asset_loader(std::unique_ptr<IAssetLoader>&& loader);
    const std::list<std::unique_ptr<IAssetLoader>>& loaders() const;
private:
    std::map<std::string, ReplacementParameterAndFilename> replacement_parameters_;
    mutable std::shared_mutex mutex_;
    std::list<std::unique_ptr<IAssetLoader>> asset_loaders_;
};

}
