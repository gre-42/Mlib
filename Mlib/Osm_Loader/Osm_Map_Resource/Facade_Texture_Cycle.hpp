#pragma once
#include <Mlib/Render/Renderables/Resource_Cycle.hpp>
#include <map>

namespace Mlib {

struct FacadeTexture;
struct Building;

class FacadeTextureCycle: public ResourceCycle<FacadeTexture> {
public:
    FacadeTextureCycle(const std::vector<FacadeTexture>& names);
    const FacadeTexture& operator () (
        const std::string& style,
        float building_top);
private:
    const FacadeTexture& from_building_top(float building_top);
    const FacadeTexture* from_style(const std::string& style);
    std::map<std::string, const FacadeTexture*> ftm_;
};

}
