#pragma once
#include <Mlib/Scene_Config/Scene_Precision.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

namespace Mlib {

inline CompressedScenePos safe_stocs(const std::string_view& s) {
	return safe_stox<CompressedScenePos>(s, "CompressedScenePos");
}

template <> inline CompressedScenePos safe_sto<CompressedScenePos>(const std::string_view& s) { return safe_stocs(s); }

}
