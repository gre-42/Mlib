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
    auto rp = ReplacementParameter::from_json(filename);
    auto mlecd = mle.changed_script_filename(filename);
    if (rp.on_init != nlohmann::detail::value_t::null) {
        mlecd(JsonView{rp.on_init}, nullptr, nullptr);
    }
    std::unique_lock lock{mutex_};
    if (!replacement_parameters_.insert({rp.id, rp}).second) {
        THROW_OR_ABORT("Asset with id \"" + rp.id + "\" already exists");
    }
}

void AssetGroupReplacementParameters::merge(const std::string& id, const JsonMacroArguments& params) {
    std::unique_lock lock{mutex_};
    auto it = replacement_parameters_.find(id);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Asset with id \"" + id + "\" does not exist");
    }
    it->second.globals.merge(params);
}

const ReplacementParameter& AssetGroupReplacementParameters::at(const std::string& id) const {
    std::shared_lock lock{mutex_};
    auto it = replacement_parameters_.find(id);
    if (it == replacement_parameters_.end()) {
        THROW_OR_ABORT("Could not find asset with id \"" + id + "\" in the asset group");
    }
    return it->second;
}

std::map<std::string, ReplacementParameter>::iterator AssetGroupReplacementParameters::begin() {
    return replacement_parameters_.begin();
}

std::map<std::string, ReplacementParameter>::iterator AssetGroupReplacementParameters::end() {
    return replacement_parameters_.end();
}
