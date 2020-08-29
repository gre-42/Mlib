#pragma once
#include <Mlib/Cpx/Renderable.hpp>
#include <list>

namespace Mlib { namespace cpx {

class Text: public Renderable {
    std::string contents_;
public:
    Text(const std::string& contents):
        contents_(contents) {}
    virtual void render(std::ostream& ostream) const override {
        ostream << contents_;
    }
};

} }
