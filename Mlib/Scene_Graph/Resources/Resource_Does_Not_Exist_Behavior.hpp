#pragma once

#undef IGNORE

namespace Mlib {

enum class ResourceDoesNotExistBehavior {
    THROW,
    RETURN_NULL
};

enum class PreloadResourceDoesNotExistBehavior {
    THROW,
    IGNORE
};

}
