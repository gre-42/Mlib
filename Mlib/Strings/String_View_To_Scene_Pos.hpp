#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Strings/String_View_To_Number.hpp>

namespace Mlib {

template <>
inline CompressedScenePos safe_stox<CompressedScenePos>(const std::string_view& s, const char* msg) {
	return (CompressedScenePos)safe_stox<ScenePos>(s, msg);
}

}
