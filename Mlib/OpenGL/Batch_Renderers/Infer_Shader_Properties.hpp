#pragma once

namespace Mlib {

struct MeshMeta;

bool get_has_per_instance_continuous_texture_layer(const MeshMeta& mesh_meta);

bool get_has_discrete_atlas_texture_layer(const MeshMeta& mesh_meta);

}
