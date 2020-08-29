#pragma once
#include <Mlib/Cpx/Renderable.hpp>
#include <Mlib/Cpx/Text.hpp>
#include <list>
#include <memory>

namespace Mlib { namespace cpx {

class Tag: public Renderable {
    std::string name_;
    std::list<std::pair<std::string, std::string>> attributes_;
    std::list<std::shared_ptr<Renderable>> contents_;
public:
    Tag(const std::string& name,
        const std::list<std::shared_ptr<Renderable>>& contents):
        name_(name),
        contents_(contents) {}
    Tag& attr(const std::string& key, const std::string& value) {
        attributes_.push_back(std::make_pair(key, value));
        return *this;
    }
    virtual void render(std::ostream& ostream) const override  {
        ostream << "<" << name_;
        if (attributes_.size() > 0) {
            ostream << " ";
            for (const std::pair<std::string, std::string>& a : attributes_) {
                ostream << a.first << "=\"" << a.second << '"';
            }
        }
        ostream << ">";
        for (const std::shared_ptr<Renderable>& c : contents_) {
            ostream << *c;
            if (contents_.size() > 1) {
                ostream << std::endl;
            }
        }
        ostream << "<" << name_ << "/>";
    }
};

}}
