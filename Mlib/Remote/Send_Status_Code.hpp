#pragma once

namespace Mlib {
    
enum class SendStatusCode: int {
    SUCCESS,
    RECONNECTING,
    TIMEOUT,
    ERROR
};

}
