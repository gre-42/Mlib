#pragma once
#include <Mlib/Os/Io/Binary_Bitwise_Words_Reader.hpp>
#include <Mlib/Os/Io/Binary_Bitwise_Words_Writer.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Read.hpp>
#include <Mlib/Os/Io/Serialize/Serialization_Context_Write.hpp>
#include <Mlib/Type_Traits/Shared_Ptr.hpp>
#include <memory>

namespace Mlib {

template <typename T, typename Archive = DefaultArchive>
concept HasLoadShared = requires(
    T& obj,
    Archive& archive
) {
    obj.load_shared(archive);
};

template <typename T>
class ConstructShared {
public:
    explicit ConstructShared(std::shared_ptr<T>& v): v_{v} {}
    void operator () (auto&&... args) {
        v_ = std::make_shared<T>(std::forward<decltype(args)>(args)...);
    }
private:
    std::shared_ptr<T>& v_;
};

template <SharedPtr TSharedPtr>
void save(
    BinaryBitwiseWordsWriter& writer,
    const TSharedPtr& value,
    std::string_view message)
{
    if (writer.ctx == nullptr) {
        throw std::runtime_error("Attempt to write a shared_ptr without a context");
    }
    auto [id, added] = writer.ctx->add_or_get(value);
    writer.write_binary(id, message);
    if (added && (value != nullptr)) {
        save(writer, *value, message);
    }
}

template <SharedPtr TSharedPtr>
void load(
    BinaryBitwiseWordsReader& reader,
    TSharedPtr& result,
    std::string_view message)
{
    if (reader.ctx == nullptr) {
        throw std::runtime_error("Attempt to load a shared_ptr without a context");
    }
    using T = TSharedPtr::element_type;
    auto id = reader.read_binary<uint32_t>("shared ptr ID");
    auto res = reader.ctx->try_get<T>(id);
    if (res.has_value()) {
        result = *res;
        return;
    }
    ReadingArchive archive{reader, message};
    if constexpr (HasLoadAndConstruct<T>) {
        ConstructShared<T> construct{result};
        T::load_and_construct(archive, construct);
    } else {
        result = std::make_shared<T>();
        load(reader, *result, message);
    }
    reader.ctx->add(result);
}

}
