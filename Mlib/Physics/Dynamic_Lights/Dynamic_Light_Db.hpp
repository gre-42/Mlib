#pragma once
#include <Mlib/Map/Threadsafe_String_Map.hpp>
#include <variant>

namespace Mlib {

struct AnimatedDynamicLightConfiguration;
struct ConstantDynamicLightConfiguration;
class IDynamicLight;

class DynamicLightDb {
    DynamicLightDb(const DynamicLightDb&) = delete;
    DynamicLightDb& operator = (const DynamicLightDb&) = delete;
public:
    using TLightConfiguration = std::variant<AnimatedDynamicLightConfiguration, ConstantDynamicLightConfiguration>;

    DynamicLightDb();
    ~DynamicLightDb();
    void add(const std::string& name, const TLightConfiguration& config);
    const TLightConfiguration& get(const std::string& name) const;
private:
    ThreadsafeStringMap<TLightConfiguration> configurations_;
};

}
