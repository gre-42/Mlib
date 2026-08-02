#pragma once

#undef ERROR

namespace Mlib {
    
enum class SendStatusCode: int {
    SUCCESS,
    RECONNECTING,
    TIMEOUT,
    ERROR
};

}
