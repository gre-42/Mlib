#pragma once
#include <Mlib/Scene_Precision.hpp>
#include <Mlib/Strings/To_Number.hpp>

namespace Mlib {

template <>
inline CompressedScenePos safe_stox<CompressedScenePos>(const std::string& s, const char* msg) {
	return (CompressedScenePos)safe_stox<ScenePos>(s, msg);
}

inline CompressedScenePos safe_stocs(const std::string& s) {
	return safe_stox<CompressedScenePos>(s, "CompressedScenePos");
}

}
