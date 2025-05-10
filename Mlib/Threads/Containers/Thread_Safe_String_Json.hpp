#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>
#include <nlohmann/json_fwd.hpp>

namespace Mlib {

void from_json(const nlohmann::json& j, ThreadSafeString& t);
void to_json(nlohmann::json& j, const ThreadSafeString& t);

}
