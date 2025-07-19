#include <Mlib/Array/Chunked_Array.hpp>
#include <Mlib/Assert.hpp>
#include <Mlib/Floating_Point_Exceptions.hpp>
#include <Mlib/List/Thread_Safe_List.hpp>
#include <Mlib/Math/Math.hpp>
#include <Mlib/Memory/Dangling_Base_Class.hpp>
#include <Mlib/Memory/Dangling_Unique_Ptr.hpp>
#include <Mlib/Memory/Destruction_Functions.hpp>
#include <Mlib/Memory/Object_Pool.hpp>
#include <Mlib/Memory/Resource_Ptr.hpp>
#include <Mlib/Os/Os.hpp>
#include <Mlib/Regex/Misc.hpp>
#include <Mlib/Regex/Template_Regex.hpp>
#include <Mlib/Threads/Dispatcher.hpp>
#include <Mlib/Threads/Recursive_Shared_Mutex.hpp>
#include <Mlib/Try_Find.hpp>
#include <iostream>

using namespace Mlib;

struct A { int a = 3; };

void test_resource_ptr() {
    resource_ptr_target<float> v{new float(5)};
    {
        resource_ptr<float> pv = v.instantiate();
        assert_isequal(*pv, 5.f);
    }
    resource_ptr_target<A> a{new A};
    {
        resource_ptr<A> b = a.instantiate();
        assert_isequal(b->a, 3);
    }
}

void test_substitute() {
    {
        std::string str = "macro_playback create-CAR_NAME SUFFIX:SUFFIX IF_WITH_PHYSICS: IF_RACING:IF_RACING IF_RALLY:IF_RALLY";
        std::string replacements = "CAR_NAME:_XZ SUFFIX:_npc1 IF_WITH_PHYSICS: IF_RACING:# IF_RALLY:";
        assert_true(substitute(str, replacements_to_map(replacements)) == "macro_playback create_XZ SUFFIX:_npc1 IF_WITH_PHYSICS: IF_RACING:# IF_RALLY:");
    }
    {
        std::string str = "IS_SMALL";
        std::string replacements = "IS_SMALL:0";
        assert_true(substitute(str, replacements_to_map(replacements)) == "0");
    }
}

struct IntShutdown {
    int value;
    inline void shutdown() {}
};

DP_IMPLEMENT(IntShutdown);

void test_dangling_unique() {
    std::list<DanglingPtr<IntShutdown>> lst;
    {
        auto a = make_dunique<IntShutdown>(5);
        auto b = a.get(DP_LOC);
        lst.push_back(a.get(DP_LOC));
        lst.clear();
        assert_true(b->value == 5);
    }
}

struct Ads {
    int i;
    Ads* ptr() { return this; }
    inline void shutdown() {}
};

DP_IMPLEMENT(Ads);

struct D {
    D(DanglingRef<Ads> p) : p{ p } {}
    DanglingRef<Ads> p;
};

void test_dangling_unique2() {
    for (size_t i = 0; i < 1000; ++i) {
        auto n = make_dunique<Ads>(5);
        D n0(DanglingRef<Ads>::from_object(*n->ptr(), DP_LOC));
        D n1(DanglingRef<Ads>::from_object(*n->ptr(), DP_LOC));
        n.get(DP_LOC);
    }
}

void test_template_regex() {
    using namespace TemplateRegex;
    auto re_hello = str("hello");
    auto re_world = str("world");
    SMatch<1> match0;
    SMatch<2> match1;
    assert_true(regex_match("hello world", match0, re_hello));
    assert_true(!regex_match("hello world", match0, re_world));
    {
        auto re_group_hello = group(re_hello);
        assert_true(regex_match("hello world", match1, re_group_hello));
        assert_true(match1[1].matched());
        assert_true(match1[1].str() == "hello");
    }
    {
        auto re_group_world = group(re_world);
        assert_true(!regex_match("hello world", match1, re_group_world));
        assert_true(!match1[1].matched());
        assert_true(match1[1].str() == "");
    }
    {
        auto re_group_world = group(star(re_hello));
        assert_true(regex_match("", match1, re_group_world));
        assert_true(match1[1].matched());
        assert_true(match1[1].str() == "");
    }
    {
        auto re_group_world = group(plus(re_hello));
        assert_true(!regex_match("", match1, re_group_world));
        assert_true(!match1[1].matched());
        assert_true(match1[1].str() == "");
    }
    {
        auto re_group_world = group(plus(re_hello));
        assert_true(regex_match("hello", match1, re_group_world));
        assert_true(match1[1].matched());
        assert_true(match1[1].str() == "hello");
    }
    {
        auto re_group_world = seq(group(plus(re_hello)), space);
        assert_true(regex_match("hello world", match1, re_group_world));
        assert_true(match1[1].matched());
        assert_true(match1[1].str() == "hello");
    }
    {
        auto re_group_world = seq(group(plus(re_hello)), no_space);
        assert_true(!regex_match("hello world", match1, re_group_world));
        assert_true(match1[1].matched());
        assert_true(match1[1].str() == "hello");
    }
    {
        using namespace std::string_view_literals;
        std::vector<std::string> ms;
        find_all_templated("hello world"sv, par(group(plus(re_hello)), group(plus(re_world)), group(space)), [&ms](const TemplateRegex::SMatch<4>& m) {
            ms.push_back(std::string{ m[1].str() });
            ms.push_back(std::string{ m[2].str() });
            ms.push_back(std::string{ m[3].str() });
            });
        assert_isequal<size_t>(ms.size(), 9);
    }
    {
        linfo() << "subst " << substitute_dollar("hello $xyz world $uv", [](std::string_view s) { return "-42-"; });
        linfo() << "subst " << substitute_dollar("../../scripts/include_all.scn.json", [](std::string_view s) { return "-42-"; });
        linfo() << "subst " << substitute_dollar("${selected_vehicle_id-_$user_name}", [](std::string_view s) { return "-42-"; });
    }
    // {
    //     auto re_group_world = seq(bdry, str("hello"), bdry);
    //     assert_true(regex_match("hello world", match, re_group_world));
    // }
}

void test_parallel_block() {
    std::atomic_int ctr = 0;
    Dispatcher dispatcher{std::chrono::milliseconds{1000}};
    std::thread t0{[&](){
        for (size_t i = 0; i < 10; ++i) {
            dispatcher.produce();
            ++ctr;
        }
    }};
    std::thread t1{[&](){
        for (size_t i = 0; i < 5; ++i) {
            dispatcher.wait_for_data();
            dispatcher.consume();
            ++ctr;
        }
    }};
    t0.join();
    t1.join();
    assert_true(ctr == 15);
}

void test_destruction_functions() {
    DestructionFunctions df;
    DestructionFunctionsRemovalTokens rt{ df, CURRENT_SOURCE_LOCATION };
    rt.add([]() { linfo() << "f"; }, CURRENT_SOURCE_LOCATION);
    df.clear();
}

struct MyClass: DanglingBaseClass {
    int a = 5;
};

struct MyDerived : public MyClass {};

struct MyContainer {
    DanglingBaseClassRef<Object> o;
};

void test_dangling_base_class() {
    MyDerived a;
    DanglingBaseClassRef<MyDerived> b{ a, CURRENT_SOURCE_LOCATION };
    // a.ref<Object>(CURRENT_SOURCE_LOCATION);
    // new MyContainer{ a.ref<Object>(CURRENT_SOURCE_LOCATION) };
    auto xx = b.ptr();
    linfo() << xx->a;
    DanglingBaseClassRef<MyClass> V{ b };
    linfo() << V->a;
}

void test_object_pool_std() {
    struct A: Object {
        int i = 5;
    };
    ObjectPool p{ InObjectPoolDestructor::CLEAR };
    auto& a = p.create<A>(CURRENT_SOURCE_LOCATION);
    linfo() << a.i;
}

void test_object_pool_unique() {
    struct A: Object {
        int i = 5;
    };
    ObjectPool p{ InObjectPoolDestructor::CLEAR };
    auto& a = p.add(std::make_unique<A>(), CURRENT_SOURCE_LOCATION);
    linfo() << a.i;
}

void test_try_find() {
    std::map<int, std::string> m;
    if (try_find(m, 42) != nullptr) {
        throw std::runtime_error("Expected nullptr");
    }
}

void test_log() {
    linfo() << "a\nbc\nd";
}

void test_atomic_recursive_shared_mutex() {
    SafeAtomicRecursiveSharedMutex m;
    std::scoped_lock lock{ m };
    std::scoped_lock lock2{ m };
}

void test_chunked_array() {
    ChunkedArray<std::list<std::vector<int>>> ar{ 3 };
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
    ar.emplace_back(5);
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
    ar.emplace_back(6);
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
    ar.emplace_back(7);
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
    ar.emplace_back(8);
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
    ar.emplace_back(9);
    for (const auto& e : ar) { linfo() << e; }; linfo() << "-";
}

void test_thread_safe_list() {
    ThreadSafeList<int> lst(3u, 42);
    lst.emplace_back(5);
    for (const auto& e : lst.scoped()) {
        linfo() << e;
    }
}

int main(int argc, const char** argv) {
    enable_floating_point_exceptions();

    try {
        test_chunked_array();
        test_thread_safe_list();
        test_resource_ptr();
        test_substitute();
        test_dangling_unique();
        test_template_regex();
        test_parallel_block();
        test_destruction_functions();
        test_dangling_base_class();
        test_object_pool_std();
        test_object_pool_unique();
        test_dangling_unique2();
        test_try_find();
        test_log();
        test_atomic_recursive_shared_mutex();
    } catch (const std::runtime_error& e) {
        lerr() << "Test failed: " << e.what();
        return 1;
    }
    return 0;
}
