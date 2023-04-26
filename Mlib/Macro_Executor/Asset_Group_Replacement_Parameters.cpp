#include "Asset_Group_Replacement_Parameters.hpp"
#include <Mlib/Macro_Executor/Macro_Line_Executor.hpp>
#include <Mlib/Macro_Executor/Replacement_Parameter.hpp>
#include <Mlib/Throw_Or_Abort.hpp>
#include <mutex>

using namespace Mlib;

AssetGroupReplacementParameters::AssetGroupReplacementParameters() = default;

AssetGroupReplacementParameters::~AssetGroupReplacementParameters() = default;

void AssetGroupReplacementParameters::insert(
    const std::string& filename,
    const MacroLineExecutor& mle)
{
    std::unique_lock lock{mutex_};
    auto rp = ReplacementParameter::from_json(filename);
    auto mlecd = mle.changed_script_filename(filename);
    for (const auto& l : rp.on_init) {
        mlecd(l, nullptr, nullptr);
    }
    if (!replacement_parameters_.insert({rp.name, rp}).second) {
        THROW_OR_ABORT("Asset with name \"" + rp.name + "\" already exists");
    }
}

const ReplacementParameter& AssetGroupReplacementParameters::at(const std::string& name) {
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(name);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find asset with name \"" + name + "\" in the asset group");
    }
    return it->second;
}
