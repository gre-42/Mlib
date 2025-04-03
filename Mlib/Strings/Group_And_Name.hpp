#pragma once
#include <iosfwd>
#include <string>
#include <string_view>

namespace Mlib {

class GroupAndName {
public:
    GroupAndName()
    : group_length_{ 0 }
    {}
    GroupAndName(const char* full_name)
    : full_name_{ full_name }
    , group_length_{ 0 }
    {}
    GroupAndName(std::string full_name)
    : full_name_{ std::move(full_name) }
    , group_length_{ 0 }
    {}
    GroupAndName(const std::string& group, const std::string& name)
    : full_name_{ group + name }
    , group_length_{ group.length() }
    {}
    inline const std::string& full_name() const {
        return full_name_;
    }
    inline std::string_view group() const {
        return std::string_view{ full_name_.data(), group_length_ };
    }
    inline std::string_view name() const {
        return std::string_view{
            full_name_.data() + group_length_,
            full_name_.length() - group_length_
        };
    }
    inline bool empty() const {
        return full_name_.empty();
    }
    GroupAndName operator + (const std::string& rhs) const {
        return { full_name() + rhs, group_length_ };
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(full_name_);
    }
private:
    GroupAndName(std::string full_name, size_t group_length)
    : full_name_{ std::move(full_name) }
    , group_length_{ group_length }
    {}
    std::string full_name_;
    size_t group_length_;
};

inline std::ostream& operator << (std::ostream& ostr, const GroupAndName& n) {
    return (ostr << n.full_name());
}

}
