#include "proctree_json.hpp"
#include <Mlib/Json/Json_View.hpp>
#include <Mlib/Misc/Argument_List.hpp>
#include <proctree/proctree.hpp>

namespace KnownArgs {
BEGIN_ARGUMENT_LIST;
DECLARE_ARGUMENT(clump_max);
DECLARE_ARGUMENT(clump_min);
DECLARE_ARGUMENT(length_falloff_factor);
DECLARE_ARGUMENT(length_falloff_power);
DECLARE_ARGUMENT(branch_factor);
DECLARE_ARGUMENT(radius_falloff_rate);
DECLARE_ARGUMENT(climb_rate);
DECLARE_ARGUMENT(trunk_kink);
DECLARE_ARGUMENT(max_radius);
DECLARE_ARGUMENT(tree_steps);
DECLARE_ARGUMENT(taper_rate);
DECLARE_ARGUMENT(twist_rate);
DECLARE_ARGUMENT(segments);
DECLARE_ARGUMENT(levels);
DECLARE_ARGUMENT(sweep_amount);
DECLARE_ARGUMENT(initial_branch_length);
DECLARE_ARGUMENT(trunk_length);
DECLARE_ARGUMENT(drop_amount);
DECLARE_ARGUMENT(grow_amount);
DECLARE_ARGUMENT(v_multiplier);
DECLARE_ARGUMENT(twig_scale);
}

using namespace Mlib;

void Proctree::from_json(const nlohmann::json& j, Proctree::Properties& properties) {
    JsonView jv{j};
    jv.validate(KnownArgs::options);
    if (auto v = jv.try_at<float>(KnownArgs::clump_max)) properties.mClumpMax = *v;
    if (auto v = jv.try_at<float>(KnownArgs::clump_min)) properties.mClumpMin = *v;
    if (auto v = jv.try_at<float>(KnownArgs::length_falloff_factor)) properties.mLengthFalloffFactor = *v;
    if (auto v = jv.try_at<float>(KnownArgs::length_falloff_power)) properties.mLengthFalloffPower = *v;
    if (auto v = jv.try_at<float>(KnownArgs::branch_factor)) properties.mBranchFactor = *v;
    if (auto v = jv.try_at<float>(KnownArgs::radius_falloff_rate)) properties.mRadiusFalloffRate = *v;
    if (auto v = jv.try_at<float>(KnownArgs::climb_rate)) properties.mClimbRate = *v;
    if (auto v = jv.try_at<float>(KnownArgs::trunk_kink)) properties.mTrunkKink = *v;
    if (auto v = jv.try_at<float>(KnownArgs::max_radius)) properties.mMaxRadius = *v;
    if (auto v = jv.try_at<int>(KnownArgs::tree_steps)) properties.mTreeSteps = *v;
    if (auto v = jv.try_at<float>(KnownArgs::taper_rate)) properties.mTaperRate = *v;
    if (auto v = jv.try_at<float>(KnownArgs::twist_rate)) properties.mTwistRate = *v;
    if (auto v = jv.try_at<int>(KnownArgs::segments)) properties.mSegments = *v;
    if (auto v = jv.try_at<int>(KnownArgs::levels)) properties.mLevels = *v;
    if (auto v = jv.try_at<float>(KnownArgs::sweep_amount)) properties.mSweepAmount = *v;
    if (auto v = jv.try_at<float>(KnownArgs::initial_branch_length)) properties.mInitialBranchLength = *v;
    if (auto v = jv.try_at<float>(KnownArgs::trunk_length)) properties.mTrunkLength = *v;
    if (auto v = jv.try_at<float>(KnownArgs::drop_amount)) properties.mDropAmount = *v;
    if (auto v = jv.try_at<float>(KnownArgs::grow_amount)) properties.mGrowAmount = *v;
    if (auto v = jv.try_at<float>(KnownArgs::v_multiplier)) properties.mVMultiplier = *v;
    if (auto v = jv.try_at<float>(KnownArgs::twig_scale)) properties.mTwigScale = *v;
}
