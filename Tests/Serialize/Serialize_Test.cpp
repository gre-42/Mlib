#include <Mlib/Misc/Floating_Point_Exceptions.hpp>
#include <Mlib/Os/Io/Serialize/Serialize.hpp>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

using namespace Mlib;

struct S {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    int x;
    int y;
};

struct S1 {
    S1(int x, int y): x{x} , y{y} {}
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    template<typename Archive, typename Construct>
    static void load_and_construct(
        Archive& archive,
        Construct& construct)
    {
        int x;
        int y;
        archive(x);
        archive(y);
        construct(x, y);
    }
    int x;
    int y;
};

struct S2: public virtual Object {
    S2(int x, int y): x{x} , y{y} {}
    template <class Archive>
    void serialize(Archive& archive) {
        archive(x);
        archive(y);
    }
    template<typename Archive, typename Construct>
    static void load_and_construct(
        Archive& archive,
        Construct& construct)
    {
        int x;
        int y;
        archive(x);
        archive(y);
        construct(x, y);
    }
    int x;
    int y;
};

struct S3 {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(s2);
    }
    std::shared_ptr<S2> s2;
};

struct S4 {
    template <class Archive>
    void serialize(Archive& archive) {
        archive(s);
    }
    std::unique_ptr<S> s;
};

void test_serialize_number() {
    std::stringstream sstr;
    {
        uint32_t a = 42;
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        uint32_t b;
        load(reader, b, "a");
        linfo() << "b: " << b;
    }
}

void test_serialize_struct() {
    std::stringstream sstr;
    {
        S a{42, 43};
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        S b;
        load(reader, b, "a");
        linfo() << "b: " << b.x << " " << b.y;
    }
}

void test_serialize_struct1() {
    std::stringstream sstr;
    {
        S1 a{42, 43};
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        SerializationContextWrite ctx;
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        auto b = load<S1>(reader, "a");
        linfo() << "b: " << b.x << " " << b.y;
    }
}

void test_serialize_struct2() {
    std::stringstream sstr;
    {
        auto a = std::make_shared<S2>(42, 43);
        SerializationContextWrite ctx;
        BinaryBitwiseWordsWriter writer{sstr, &ctx};
        save(writer, a, "a");
    }
    {
        SerializationContextRead ctx;
        BinaryBitwiseWordsReader reader{sstr, &ctx, IoVerbosity::SILENT};
        std::shared_ptr<S2> b;
        load(reader, b, "a");
        linfo() << "b: " << b->x << " " << b->y;
    }
}

void test_serialize_struct3() {
    {
        std::stringstream sstr;
        {
            S3 a{std::make_shared<S2>(42, 43)};
            SerializationContextWrite ctx;
            BinaryBitwiseWordsWriter writer{sstr, &ctx};
            save(writer, a, "a");
        }
        {
            SerializationContextRead ctx;
            BinaryBitwiseWordsReader reader{sstr, &ctx, IoVerbosity::SILENT};
            S3 b;
            load(reader, b, "a");
            linfo() << "b: " << b.s2->x << " " << b.s2->y;
        }
    }
    {
        std::stringstream sstr;
        {
            S3 a{nullptr};
            SerializationContextWrite ctx;
            BinaryBitwiseWordsWriter writer{sstr, &ctx};
            save(writer, a, "a");
        }
        {
            SerializationContextRead ctx;
            BinaryBitwiseWordsReader reader{sstr, &ctx, IoVerbosity::SILENT};
            S3 b;
            load(reader, b, "a");
            linfo() << "b: " << b.s2.get();
        }
    }
}

void test_serialize_struct4() {
    std::stringstream sstr;
    {
        S4 a{std::make_unique<S>(42, 43)};
        SerializationContextWrite ctx;
        BinaryBitwiseWordsWriter writer{sstr, &ctx};
        save(writer, a, "a");
    }
    {
        SerializationContextRead ctx;
        BinaryBitwiseWordsReader reader{sstr, &ctx, IoVerbosity::SILENT};
        S4 b{std::make_unique<S>()};
        load(reader, b, "a");
        linfo() << "b: " << b.s->x << " " << b.s->y;
    }
}

void test_serialize_map() {
    using Map = std::map<std::string, std::vector<int>>;
    std::stringstream sstr;
    {
        Map a{{"x", {42, 43}}, {"y", {44, 45}}};
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        Map b;
        load(reader, b, "a");
        linfo() << "b: " << b.at("x").at(0) << " " << b.at("y").at(1);
    }
}

void test_serialize_map2() {
    using Map = std::unordered_map<std::string, std::set<int>>;
    std::stringstream sstr;
    {
        Map a{{"x", {42, 43}}, {"y", {44, 45}}};
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        Map b;
        load(reader, b, "a");
        linfo() << "b: " << (int)b.at("x").contains(42) << " " << (int)b.at("y").contains(45);
    }
}

void test_duration() {
    std::stringstream sstr;
    {
        std::chrono::steady_clock::duration a{123};
        BinaryBitwiseWordsWriter writer{sstr, nullptr};
        save(writer, a, "a");
    }
    {
        BinaryBitwiseWordsReader reader{sstr, nullptr, IoVerbosity::SILENT};
        std::chrono::steady_clock::duration b;
        load(reader, b, "a");
        linfo() << "b: " << (int)b.count();
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    try {
        test_serialize_number();
        test_serialize_struct();
        test_serialize_struct1();
        test_serialize_struct2();
        test_serialize_struct3();
        test_serialize_struct4();
        test_serialize_map();
        test_serialize_map2();
        test_duration();
    } catch (const std::exception& e) {
        lerr() << "Test failed: " << e.what();
        return 1;
    }
    return 0;
}
