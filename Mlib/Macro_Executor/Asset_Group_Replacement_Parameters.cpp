#include "Asset_Group_Replacement_Parameters.hpp"
#include <Mlib/Geometry/Interfaces/IAsset_Loader.hpp>
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

AssetGroupReplacementParameters::AssetGroupReplacementParameters() = default;

AssetGroupReplacementParameters::~AssetGroupReplacementParameters() = default;

void AssetGroupReplacementParameters::insert_if_active(
    const std::string& filename,
    const MacroLineExecutor& mle)
{
    auto rp = ReplacementParameterAndFilename::from_json(filename);
    if (!mle.eval(rp.rp.required.fixed)) {
        return;
    }
    auto mlecd = mle.changed_script_filename(filename);
    if (rp.rp.on_init != nlohmann::detail::value_t::null) {
        mlecd(rp.rp.on_init, nullptr);
    }
    insert(std::move(rp));
}

void AssetGroupReplacementParameters::insert(ReplacementParameterAndFilename&& rp) {
    std::unique_lock lock{mutex_};
    if (!replacement_parameters_.try_emplace(rp.rp.id, std::move(rp)).second) {
        THROW_OR_ABORT("Asset with id \"" + rp.rp.id + "\" already exists");
    }
}

void AssetGroupReplacementParameters::merge_into_database(const std::string& id, const JsonMacroArguments& params) {
    std::unique_lock lock{mutex_};
    auto it = replacement_parameters_.find(id);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Asset with id \"" + id + "\" does not exist");
    }
    it->second.rp.database.merge(params);
}

const ReplacementParameterAndFilename& AssetGroupReplacementParameters::at(const std::string& id) const {
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(id);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find asset with id \"" + id + "\" in the asset group");
    }
    return it->second;
}

std::map<std::string, ReplacementParameterAndFilename>::const_iterator AssetGroupReplacementParameters::begin() const {
    return replacement_parameters_.begin();
}

std::map<std::string, ReplacementParameterAndFilename>::const_iterator AssetGroupReplacementParameters::end() const {
    return replacement_parameters_.end();
}

void AssetGroupReplacementParameters::add_asset_loader(std::unique_ptr<IAssetLoader>&& loader) {
    std::shared_lock lock{mutex_};
    asset_loaders_.emplace_back(std::move(loader));
}

const std::list<std::unique_ptr<IAssetLoader>>& AssetGroupReplacementParameters::loaders() const {
    std::shared_lock lock{mutex_};
    return asset_loaders_;
}
