#pragma once
#include <Mlib/Scene_Graph/Elements/Scene_Node.hpp>
#include <Mlib/Scene_Graph/Interfaces/Scene_Node/IAbsolute_Movable.hpp>
#include <Mlib/Scene_Graph/Status_Writer.hpp>
#include <Mlib/Throw_Or_Abort.hpp>

namespace Mlib {

inline StatusWriter& get_status_writer(const DanglingBaseClassRef<SceneNode>& node) {
    auto sw = dynamic_cast<StatusWriter*>(&node->get_absolute_movable());
    if (sw == nullptr) {
        THROW_OR_ABORT("Absolute movable is not a status-writer");
    }
    return *sw;
}

}
