#pragma once
#include <Mlib/Geometry/Material.hpp>
#include <Mlib/Geometry/Modifier_Backlog.hpp>
#include <Mlib/Geometry/Morphology.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <Mlib/Strings/Group_And_Name.hpp>

namespace Mlib {

struct MeshMeta {
    GroupAndName name;
    Material material;
    Morphology morphology;
    ModifierBacklog modifier_backlog;
    template <class Archive>
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(name);
        archive(material);
        archive(morphology);
        archive(modifier_backlog);
    }
};

}
