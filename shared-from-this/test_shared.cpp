#include "shared.h"

#include <catch.hpp>

#include "allocations_checker.h"

#include <memory>

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Empty") {
    SECTION("Empty state") {
        SharedPtr<int> a, b;

        b = a;
        SharedPtr c(a);
        b = std::move(c);

        REQUIRE(a.Get() == nullptr);
        REQUIRE(b.Get() == nullptr);
        REQUIRE(c.Get() == nullptr);
    }

    SECTION("No allocations in default ctor") {
        EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>());
        EXPECT_ZERO_ALLOCATIONS(SharedPtr<int>(nullptr));
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

TEST_CASE("Copy/move") {
    SharedPtr<std::string> a(new std::string("aba"));
    std::string* ptr;
    {
        SharedPtr b(a);
        SharedPtr c(a);
        ptr = c.Get();
    }
    REQUIRE(ptr == a.Get());
    REQUIRE(*ptr == "aba");

    SharedPtr<std::string> b(new std::string("caba"));
    {
        SharedPtr c(b);
        SharedPtr d(b);
        d = std::move(a);
        REQUIRE(*c == "caba");
        REQUIRE(*d == "aba");
        b.Reset(new std::string("test"));
        REQUIRE(*c == "caba");
    }
    REQUIRE(*b == "test");

    SharedPtr<std::string> end;
    {
        SharedPtr<std::string> d(new std::string("delete"));
        d = b;
        SharedPtr c(std::move(b));
        REQUIRE(*d == "test");
        REQUIRE(*c == "test");
        d = d;
        c = end;
        d.Reset(new std::string("delete"));
        end = d;
    }

    REQUIRE(*end == "delete");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct ModifiersB {
    static int count;

    ModifiersB() {
        ++count;
    }
    ModifiersB(const ModifiersB&) {
        ++count;
    }
    virtual ~ModifiersB() {
        --count;
    }
};

int ModifiersB::count = 0;

struct ModifiersA : public ModifiersB {
    static int count;

    ModifiersA() {
        ++count;
    }
    ModifiersA(const ModifiersA& other) : ModifiersB(other) {
        ++count;
    }
    ~ModifiersA() {
        --count;
    }
};

int ModifiersA::count = 0;

struct ModifiersC {
    static int count;

    ModifiersC() {
        ++count;
    }
    ModifiersC(const ModifiersC&) {
        ++count;
    }
    ~ModifiersC() {
        --count;
    }
};

int ModifiersC::count = 0;

TEST_CASE("Modifiers") {
    SECTION("Reset") {
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            p.Reset();
            REQUIRE(ModifiersA::count == 0);
            REQUIRE(ModifiersB::count == 0);
            REQUIRE(p.UseCount() == 0);
            REQUIRE(p.Get() == nullptr);
        }
        REQUIRE(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            p.Reset();
            REQUIRE(ModifiersA::count == 0);
            REQUIRE(ModifiersB::count == 0);
            REQUIRE(p.UseCount() == 0);
            REQUIRE(p.Get() == nullptr);
        }
        REQUIRE(ModifiersA::count == 0);
    }

    SECTION("Reset ptr") {
        {
            SharedPtr<ModifiersB> p(new ModifiersB);
            ModifiersA* ptr = new ModifiersA;
            p.Reset(ptr);
            REQUIRE(ModifiersA::count == 1);
            REQUIRE(ModifiersB::count == 1);
            REQUIRE(p.UseCount() == 1);
            REQUIRE(p.Get() == ptr);
        }
        REQUIRE(ModifiersA::count == 0);
        {
            SharedPtr<ModifiersB> p;
            ModifiersA* ptr = new ModifiersA;
            p.Reset(ptr);
            REQUIRE(ModifiersA::count == 1);
            REQUIRE(ModifiersB::count == 1);
            REQUIRE(p.UseCount() == 1);
            REQUIRE(p.Get() == ptr);
        }
        REQUIRE(ModifiersA::count == 0);
    }

    SECTION("Swap") {
        {
            ModifiersC* ptr1 = new ModifiersC;
            ModifiersC* ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 1);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 1);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 2);
            }
            REQUIRE(p1.UseCount() == 1);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 1);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = new ModifiersC;
            ModifiersC* ptr2 = nullptr;
            SharedPtr<ModifiersC> p1(ptr1);
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 0);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 1);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 1);
            }
            REQUIRE(p1.UseCount() == 0);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 0);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = nullptr;
            ModifiersC* ptr2 = new ModifiersC;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2(ptr2);
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 1);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 0);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 1);
            }
            REQUIRE(p1.UseCount() == 1);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 1);
        }
        REQUIRE(ModifiersC::count == 0);
        {
            ModifiersC* ptr1 = nullptr;
            ModifiersC* ptr2 = nullptr;
            SharedPtr<ModifiersC> p1;
            {
                SharedPtr<ModifiersC> p2;
                p1.Swap(p2);
                REQUIRE(p1.UseCount() == 0);
                REQUIRE(p1.Get() == ptr2);
                REQUIRE(p2.UseCount() == 0);
                REQUIRE(p2.Get() == ptr1);
                REQUIRE(ModifiersC::count == 0);
            }
            REQUIRE(p1.UseCount() == 0);
            REQUIRE(p1.Get() == ptr2);
            REQUIRE(ModifiersC::count == 0);
        }
        REQUIRE(ModifiersC::count == 0);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct OperatorBoolA {
    int a;
    virtual ~OperatorBoolA(){};
};

TEST_CASE("Observers") {
    SECTION("operator->") {
        const SharedPtr<std::pair<int, int>> p(new std::pair<int, int>(3, 4));
        REQUIRE(p->first == 3);
        REQUIRE(p->second == 4);
        p->first = 5;
        p->second = 6;
        REQUIRE(p->first == 5);
        REQUIRE(p->second == 6);
    }

    SECTION("Dereference") {
        const SharedPtr<int> p(new int(32));
        REQUIRE(*p == 32);
        *p = 3;
        REQUIRE(*p == 3);
    }

    SECTION("operator bool") {
        static_assert(std::is_constructible<bool, SharedPtr<OperatorBoolA>>::value, "");
        static_assert(!std::is_convertible<SharedPtr<OperatorBoolA>, bool>::value, "");
        {
            const SharedPtr<int> p(new int(32));
            REQUIRE(p);
        }
        {
            const SharedPtr<int> p;
            REQUIRE(!p);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

struct Pinned {
    Pinned(int tag) : tag_(tag) {
    }

    Pinned(const Pinned& a) = delete;
    Pinned(Pinned&& a) = delete;

    Pinned& operator=(const Pinned& a) = delete;
    Pinned& operator=(Pinned&& a) = delete;

    ~Pinned() = default;

    int GetTag() const {
        return tag_;
    }

private:
    int tag_;
};

TEST_CASE("No copies") {
    SharedPtr<Pinned> p(new Pinned(1));
}

struct D {
    D(Pinned& pinned, std::unique_ptr<int>&& p)
        : some_uncopyable_thing_(std::move(p)), pinned_(pinned) {
    }

    int GetUP() const {
        return *some_uncopyable_thing_;
    }

    Pinned& GetPinned() const {
        return pinned_;
    }

private:
    std::unique_ptr<int> some_uncopyable_thing_;
    Pinned& pinned_;
};

struct Throwing {
    Throwing() {
        throw 42;
    }
};

TEST_CASE("MakeShared") {
    SECTION("One allocation") {
        EXPECT_ONE_ALLOCATION(REQUIRE(*MakeShared<int>(42) == 42));
    }

    SECTION("Parameters passing") {
        auto p_int = std::make_unique<int>(42);
        Pinned pinned(1312);
        auto p = MakeShared<D>(pinned, std::move(p_int));

        REQUIRE(p->GetUP() == 42);
        REQUIRE(p->GetPinned().GetTag() == 1312);
    }

    SECTION("Constructed only once") {
        auto sp = MakeShared<Pinned>(1);
    }

    SECTION("Faulty constructor") {
        try {
            auto sp = MakeShared<Throwing>();
        } catch (...) {
        }
    }
}

struct Data {
    static bool data_was_deleted;

    int x;
    double y;

    ~Data() {
        data_was_deleted = true;
    }
};

bool Data::data_was_deleted = false;

TEST_CASE("Aliasing constructor") {
    SECTION("It just exists") {
        SharedPtr<Data> sp(new Data{42, 3.14});

        SharedPtr<double> sp2(sp, &sp->y);

        REQUIRE(*sp2 == 3.14);
    }

    SECTION("Lifetime extension") {
        {
            Data::data_was_deleted = false;
            SharedPtr<double> sp3;
            {
                SharedPtr<Data> sp(new Data{42, 3.14});
                SharedPtr<double> sp2(sp, &sp->y);
                sp3 = sp2;
            }
            REQUIRE(*sp3 == 3.14);
            REQUIRE(!Data::data_was_deleted);
        }
        REQUIRE(Data::data_was_deleted);
    }
}

class Base {
public:
    virtual ~Base() = default;
};

class Derived : public Base {
public:
    static bool i_was_deleted;

    ~Derived() {
        i_was_deleted = true;
    }
};

bool Derived::i_was_deleted = false;

TEST_CASE("Type conversions") {
    SECTION("Destruction") {
        Derived::i_was_deleted = false;
        { SharedPtr<Base> sb(new Derived); }
        REQUIRE(Derived::i_was_deleted);
    }

    SECTION("Constness") {
        SharedPtr<int> s1(new int(42));
        SharedPtr<const int> s2 = s1;

        SharedPtr<const int> s3 = std::move(s1);
        REQUIRE(!s1);
        REQUIRE(s2.UseCount() == 2);

        s1.Reset(new int(43));
        s2 = s1;
        s3 = std::move(s1);
        REQUIRE(!s1);
        REQUIRE(s3.UseCount() == 2);
    }
}

struct A {
    ~A() = default;
};

struct B : A {
    ~B() {
        destructor_called = true;
    }

    static bool destructor_called;
};

bool B::destructor_called = false;

TEST_CASE("Destructor for correct type") {
    SECTION("Regular constructor") {
        B::destructor_called = false;
        { SharedPtr<A>(new B()); }
        REQUIRE(B::destructor_called);
    }

    SECTION("MakeShared") {
        B::destructor_called = false;
        { SharedPtr<A> ptr = MakeShared<B>(); }
        REQUIRE(B::destructor_called);
    }

    SECTION("Reset") {
        B::destructor_called = false;
        {
            SharedPtr<A> ptr(new A);
            ptr.Reset(new B);
        }
        REQUIRE(B::destructor_called);
    }
}
