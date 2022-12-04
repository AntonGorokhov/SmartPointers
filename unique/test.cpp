#include "unique.h"

#include "deleters.h"

#include <common/my_int.h>

#include <catch.hpp>
#include <vector>
#include <tuple>

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Person {
    virtual int GetFavoriteNumber() const = 0;
    virtual ~Person() = default;
};

struct Alice : Person {
    int GetFavoriteNumber() const override {
        return 37;
    }
};

struct Bob : Person {
    int GetFavoriteNumber() const override {
        return 43;
    }
};

TEST_CASE("Basic") {
    SECTION("Lifetime") {
        {
            UniquePtr<MyInt> s(new MyInt);

            REQUIRE(MyInt::AliveCount() == 1);
        }

        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Cannot copy") {
        static_assert(!std::is_copy_constructible_v<UniquePtr<int>> &&
                      !std::is_copy_assignable_v<UniquePtr<int>>);
    }

    SECTION("Noexcept") {
        static_assert(std::is_nothrow_move_constructible_v<UniquePtr<int>>);
        static_assert(std::is_nothrow_move_assignable_v<UniquePtr<int>>);
    }

    SECTION("Default value") {
        UniquePtr<int> s;

        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Move") {
        UniquePtr<int> s1(new int);
        UniquePtr<int> s2(new int);

        int* p = s1.Get();
        s2 = std::move(s1);

        REQUIRE(s2.Get() == p);
        REQUIRE(s1.Get() == nullptr);
    }

    SECTION("Self move") {
        UniquePtr<MyInt> s(new MyInt(42));
        MyInt* p = s.Get();
        s = std::move(s);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == p);
        REQUIRE(*p == 42);
    }

    SECTION("NULL") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        s = NULL;  // NOLINT

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Nullptr") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        s = nullptr;

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Modifiers") {
    SECTION("Release") {
        UniquePtr<MyInt> s(new MyInt(42));
        MyInt* ps = s.Get();
        MyInt* p = s.Release();

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == nullptr);
        REQUIRE(ps == p);
        REQUIRE(*p == 42);

        delete p;

        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Swap") {
        MyInt* p1 = new MyInt(1);
        UniquePtr<MyInt> s1(p1);
        MyInt* p2 = new MyInt(2);
        UniquePtr<MyInt> s2(p2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);

        s1.Swap(s2);

        REQUIRE(s1.Get() == p2);
        REQUIRE(*s1 == 2);
        REQUIRE(s2.Get() == p1);
        REQUIRE(*s2 == 1);
        REQUIRE(MyInt::AliveCount() == 2);

        std::swap(s1, s2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);
    }

    SECTION("Reset") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        MyInt* new_value = new MyInt;

        REQUIRE(MyInt::AliveCount() == 2);

        s.Reset(new_value);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == new_value);
    }

    SECTION("Reset const") {
        UniquePtr<const MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        const MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        MyInt* new_value = new MyInt;

        REQUIRE(MyInt::AliveCount() == 2);

        s.Reset(new_value);

        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s.Get() == new_value);
    }

    SECTION("Reset nullptr") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        s.Reset(nullptr);

        REQUIRE(MyInt::AliveCount() == 0);
        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Reset no arg") {
        UniquePtr<MyInt> s(new MyInt);

        REQUIRE(MyInt::AliveCount() == 1);

        MyInt* p = s.Get();

        REQUIRE(p != nullptr);

        s.Reset();

        REQUIRE(s.Get() == nullptr);
    }

    SECTION("Reset self pass") {
        struct Sui {
            UniquePtr<Sui> ptr_;

            Sui() : ptr_(this) {
            }

            void Reset() {
                ptr_.Reset();
            }
        };

        (new Sui)->Reset();
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Observers") {
    SECTION("Dereference") {
        UniquePtr<int> p(new int(3));

        REQUIRE(*p == 3);
    }

    SECTION("operator bool") {
        UniquePtr<int> p(new int(1));
        UniquePtr<int> const& cp = p;

        REQUIRE(p);
        REQUIRE(cp);

        UniquePtr<int> p0;
        UniquePtr<int> const& cp0 = p0;

        REQUIRE(!p0);
        REQUIRE(!cp0);
    }

    SECTION("Get") {
        int* p = new int(1);

        UniquePtr<int> s(p);
        UniquePtr<int> const& sc = s;

        REQUIRE(s.Get() == p);
        REQUIRE(sc.Get() == s.Get());
    }

    SECTION("Get const") {
        const int* p = new int(1);

        UniquePtr<const int> s(p);
        UniquePtr<const int> const& sc = s;

        REQUIRE(s.Get() == p);
        REQUIRE(sc.Get() == s.Get());
    }

    SECTION("operator->") {
        struct A {
            int i_{7};
        };

        UniquePtr<A> p(new A);
        REQUIRE(p->i_ == 7);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Construction with deleters") {
    SECTION("From copyable deleter") {
        const CopyableDeleter<MyInt> cd;
        UniquePtr<MyInt, CopyableDeleter<MyInt>> s(new MyInt, cd);
    }

    SECTION("From move-only deleter") {
        Deleter<MyInt> d;
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, std::move(d));
    }

    SECTION("From temporary") {
        UniquePtr<MyInt, Deleter<MyInt>> s(new MyInt, Deleter<MyInt>{});
    }

    SECTION("Default deleter should support upcasts") {
        using AliceDefaultDelete = std::decay_t<decltype(UniquePtr<Alice>{}.GetDeleter())>;
        using PersonDefaultDelete = std::decay_t<decltype(UniquePtr<Person>{}.GetDeleter())>;

        AliceDefaultDelete d1, d3;
        PersonDefaultDelete d2(std::move(d1));
        d2 = std::move(d3);
    }
}

TEST_CASE("Swap with deleters") {
    SECTION("If storing deleter by value") {
        MyInt* p1 = new MyInt(1);
        UniquePtr<MyInt, Deleter<MyInt>> s1(p1, Deleter<MyInt>(1));
        MyInt* p2 = new MyInt(2);
        UniquePtr<MyInt, Deleter<MyInt>> s2(p2, Deleter<MyInt>(2));

        s1.Swap(s2);

        REQUIRE(s1.Get() == p2);
        REQUIRE(*s1 == 2);
        REQUIRE(s2.Get() == p1);
        REQUIRE(*s2 == 1);
        REQUIRE(s1.GetDeleter().GetTag() == 2);
        REQUIRE(s2.GetDeleter().GetTag() == 1);
        REQUIRE(MyInt::AliveCount() == 2);

        std::swap(s1, s2);

        REQUIRE(s1.Get() == p1);
        REQUIRE(*s1 == 1);
        REQUIRE(s2.Get() == p2);
        REQUIRE(*s2 == 2);
        REQUIRE(s1.GetDeleter().GetTag() == 1);
        REQUIRE(s2.GetDeleter().GetTag() == 2);
    }
}

TEST_CASE("Moving deleters") {
    SECTION("Move with custom deleter") {
        UniquePtr<MyInt, Deleter<MyInt>> s1(new MyInt, Deleter<MyInt>(5));
        MyInt* p = s1.Get();
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);

        REQUIRE(MyInt::AliveCount() == 2);
        REQUIRE(s1.GetDeleter().GetTag() == 5);
        REQUIRE(s2.GetDeleter().GetTag() == 0);

        s2 = std::move(s1);

        REQUIRE(s2.Get() == p);
        REQUIRE(s1.Get() == nullptr);
        REQUIRE(MyInt::AliveCount() == 1);
        REQUIRE(s2.GetDeleter().GetTag() == 5);
        REQUIRE(s1.GetDeleter().GetTag() == 0);
    }
}

TEST_CASE("GetDeleter") {
    SECTION("Get deleter") {
        UniquePtr<MyInt, Deleter<MyInt>> p;

        REQUIRE(!p.GetDeleter().IsConst());
    }

    SECTION("Get deleter const") {
        const UniquePtr<MyInt, Deleter<MyInt>> p;

        REQUIRE(p.GetDeleter().IsConst());
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct VoidPtrDeleter {
    void operator()(void* ptr) {
        free(ptr);
    }
};

TEST_CASE("UniquePtr<void, VoidPtrDeleter>") {
    SECTION("It compiles!") {
        UniquePtr<void, VoidPtrDeleter> p(malloc(100));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Array specialization") {
    SECTION("delete[] is called") {
        UniquePtr<MyInt[]> u(new MyInt[100]);
        REQUIRE(MyInt::AliveCount() == 100);
        u.Reset();
        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Able to use custom deleters") {
        UniquePtr<MyInt[], Deleter<MyInt[]>> u(new MyInt[100]);
        REQUIRE(MyInt::AliveCount() == 100);
        u.Reset();
        REQUIRE(MyInt::AliveCount() == 0);
    }

    SECTION("Operator []") {
        int* arr = new int[5];
        for (size_t i = 0; i < 5; ++i) {
            arr[i] = i;
        }

        UniquePtr<int[]> u(arr);
        for (int i = 0; i < 5; ++i) {
            REQUIRE(u[i] == i);
            u[i] = -i;
            REQUIRE(u[i] == -i);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
void DeleteFunction(T* ptr) {
    delete ptr;
}

template <typename T>
struct StatefulDeleter {
    int some_useless_field = 0;

    void operator()(T* ptr) {
        delete ptr;
        ++some_useless_field;
    }
};

TEST_CASE("Compressed pair usage") {

    SECTION("Stateless struct deleter") {
        static_assert(sizeof(UniquePtr<int>) == sizeof(void*));
        static_assert(sizeof(UniquePtr<int, std::default_delete<int>>) == sizeof(int*));  // NOLINT
    }

    SECTION("Stateful struct deleter") {
        static_assert(sizeof(UniquePtr<int, StatefulDeleter<int>>) ==
                      sizeof(std::pair<int*, StatefulDeleter<int>>));
    }

    SECTION("Stateless lambda deleter") {
        auto lambda_deleter = [](int* ptr) { delete ptr; };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) == sizeof(int*));
    }

    SECTION("Stateful lambda deleter") {
        int some_useless_counter = 0;
        auto lambda_deleter = [&some_useless_counter](int* ptr) {
            delete ptr;
            ++some_useless_counter;
        };
        static_assert(sizeof(UniquePtr<int, decltype(lambda_deleter)>) ==
                      sizeof(std::pair<int*, decltype(lambda_deleter)>));
    }

    SECTION("Function pointer deleter") {
        static_assert(sizeof(UniquePtr<int, decltype(&DeleteFunction<int>)>) ==
                      sizeof(std::pair<int*, decltype(&DeleteFunction<int>)>));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename T>
class DerivedDeleter : public Deleter<T> {};

TEST_CASE("Upcasts") {
    SECTION("Upcast ptr in move constructor") {
        std::vector<UniquePtr<Person>> v;
        UniquePtr<Alice> alice(new Alice);
        v.push_back(std::move(alice));
        v.emplace_back(new Bob);
        std::vector<int> res;
        for (const auto& ptr : v) {
            res.push_back(ptr->GetFavoriteNumber());
        }
        REQUIRE(res == std::vector<int>{37, 43});
    }

    SECTION("Upcast ptr in move assignment") {
        UniquePtr<Alice> alice(new Alice);

        UniquePtr<Person> person;
        person = std::move(alice);

        REQUIRE(alice.Get() == nullptr);
        REQUIRE(person.Get() != nullptr);
        REQUIRE(person->GetFavoriteNumber() == 37);
    }

    SECTION("Upcast deleter in move constructor") {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(std::move(s));
    }

    SECTION("Upcast deleter in move assignment") {
        UniquePtr<MyInt, DerivedDeleter<MyInt>> s(new MyInt);
        UniquePtr<MyInt, Deleter<MyInt>> s2(new MyInt);
        s2 = std::move(s);
    }
}
