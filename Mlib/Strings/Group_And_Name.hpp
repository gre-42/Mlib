#pragma once
#include <Mlib/Hashing/Variable_And_Hash.hpp>
#include <Mlib/Memory/Integral_Cast.hpp>
#include <Mlib/Os/Io/Safe_Archiver.hpp>
#include <cereal/access.hpp>
#include <cstdint>
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
        , group_length_{ integral_cast<uint32_t>(group.length()) }
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
    void serialize(Archive& archiver) {
        SafeArchiver archive{archiver};
        archive(full_name_);
        archive(group_length_);
    }
    // From: https://github.com/USCiLab/cereal/issues/102
    template<typename Archive>
    static void load_and_construct(
        Archive& archiver,
        cereal::construct<GroupAndName>& construct)
    {
        SafeArchiver archive{archiver};
        std::string full_name;
        uint32_t group_length;

        archive(full_name);
        archive(group_length);

        construct(full_name, group_length);
    }
private:
    GroupAndName(VariableAndHash<std::string> full_name, uint32_t group_length)
        : full_name_{ std::move(full_name) }
        , group_length_{ group_length }
    {
        if (group_length_ > full_name_->length()) {
            throw std::runtime_error("Group length too large");
        }
    }
    VariableAndHash<std::string> full_name_;
    uint32_t group_length_;
};

inline std::ostream& operator << (std::ostream& ostr, const GroupAndName& n) {
    return (ostr << n.full_name());
}

}
