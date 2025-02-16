#include <Mlib/Json/Base.hpp>
#include <Mlib/Threads/Containers/Thread_Safe_String.hpp>

namespace Mlib {

void from_json(const nlohmann::json& j, ThreadSafeString& t);
void to_json(nlohmann::json& j, const ThreadSafeString& t);

}
