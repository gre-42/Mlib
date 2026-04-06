#pragma once
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <stdexcept>

namespace Mlib {

inline DanglingBaseClassRef<StatusWriter> get_status_writer(SceneNode& node, SourceLocation loc) {
    auto am = node.get_absolute_movable(loc);
    auto sw = dynamic_cast<StatusWriter*>(&am.get());
    if (sw == nullptr) {
        throw std::runtime_error("Absolute movable is not a status-writer");
    }
    return {*sw, loc};
}

}
