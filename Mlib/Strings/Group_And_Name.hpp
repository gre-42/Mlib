#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <cereal/access.hpp>
#include <iosfwd>
#include <stdexcept>
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
    GroupAndName(VariableAndHash<std::string> full_name)
        : full_name_{ std::move(full_name) }
        , group_length_{ 0 }
    {}
    GroupAndName(const std::string& group, const std::string& name)
        : full_name_{ group + name }
        , group_length_{ group.length() }
    {}
    inline const std::string& full_name() const {
        return *full_name_;
    }
    const VariableAndHash<std::string>& full_name_and_hash() const {
        return full_name_;
    }
    inline std::string_view group() const {
        return std::string_view{ full_name_->data(), group_length_ };
    }
    inline std::string_view name() const {
        return std::string_view{
            full_name_->data() + group_length_,
            full_name_->length() - group_length_
        };
    }
    inline bool empty() const {
        return full_name_->empty();
    }
    GroupAndName operator + (const std::string& rhs) const {
        return { VariableAndHash<std::string>{full_name() + rhs}, group_length_ };
    }
    template <class Archive>
    void serialize(Archive& archive) {
        archive(full_name_);
        archive(group_length_);
    }
    // From: https://github.com/USCiLab/cereal/issues/102
    template<typename Archive>
    static void load_and_construct(
        Archive& archive,
        cereal::construct<GroupAndName>& construct)
    {
        std::string full_name;
        size_t group_length;

        archive(full_name);
        archive(group_length);

        construct(full_name, group_length);
    }
private:
    GroupAndName(VariableAndHash<std::string> full_name, size_t group_length)
        : full_name_{ std::move(full_name) }
        , group_length_{ group_length }
    {
        if (group_length_ > full_name_->length()) {
            throw std::runtime_error("Group length too large");
        }
    }
    VariableAndHash<std::string> full_name_;
    size_t group_length_;
};

inline std::ostream& operator << (std::ostream& ostr, const GroupAndName& n) {
    return (ostr << n.full_name());
}

}
