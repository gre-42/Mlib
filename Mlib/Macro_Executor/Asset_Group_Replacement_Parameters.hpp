#pragma once
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <list>
#include <map>
#include <memory>
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
    void insert_if_active(const std::string& filename, const MacroLineExecutor& mle);
    void insert(ReplacementParameterAndFilename&& rp);
    void merge_into_database(const std::string& id, const JsonMacroArguments& params);
    const ReplacementParameterAndFilename& at(const std::string& id) const;
    std::map<std::string, ReplacementParameterAndFilename>::const_iterator begin() const;
    std::map<std::string, ReplacementParameterAndFilename>::const_iterator end() const;
    void add_asset_loader(std::unique_ptr<IAssetLoader>&& loader);
    const std::list<std::unique_ptr<IAssetLoader>>& loaders() const;
private:
    std::map<std::string, ReplacementParameterAndFilename> replacement_parameters_;
    mutable SafeAtomicRecursiveSharedMutex mutex_;
    std::list<std::unique_ptr<IAssetLoader>> asset_loaders_;
};

}
