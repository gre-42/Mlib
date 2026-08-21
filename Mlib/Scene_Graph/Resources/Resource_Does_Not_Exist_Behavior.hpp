#pragma once

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
