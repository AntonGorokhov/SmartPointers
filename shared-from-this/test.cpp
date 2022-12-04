#include "shared.h"
#include "weak.h"

#include <catch.hpp>

struct T : public EnableSharedFromThis<T> {};

struct Y : T {};

struct Z : Y {};

void NullDeleter(void*) {
}

struct Foo : virtual public EnableSharedFromThis<Foo> {
    virtual ~Foo() {
    }
};

struct Bar : public Foo {
    Bar(int) {
    }
};

TEST_CASE("SharedFromThis") {
    {
        SharedPtr<T> t1(new T);
        SharedPtr<T> t2(MakeShared<T>());
    }

    {
        int x = 42;
        SharedPtr<Bar> t1(new Bar(42));
        REQUIRE(t1->SharedFromThis() == t1);
        SharedPtr<Bar> t2(MakeShared<Bar>(x));
        REQUIRE(t2->SharedFromThis() == t2);
    }

    {
        SharedPtr<Y> p(new Z);
        SharedPtr<T> q = p->SharedFromThis();
        REQUIRE(p == q);
    }

    {
        T* ptr = new T;
        SharedPtr<T> s(ptr);
        REQUIRE(!ptr->WeakFromThis().Expired());
        {
            try {
                SharedPtr<T> new_s = ptr->SharedFromThis();
                REQUIRE(new_s == s);
            } catch (...) {
                REQUIRE(false);
            }
        }
        s.Reset();
    }

    {
        T* ptr = new T;
        WeakPtr<T> weak;
        {
            SharedPtr<T> s(ptr);
            REQUIRE(ptr->SharedFromThis() == s);
            weak = s;
            REQUIRE(!weak.Expired());
        }
        REQUIRE(weak.Expired());
        weak.Reset();
    }
}

TEST_CASE("WeakFromThis") {
    T* ptr = new T;
    const T* cptr = ptr;

    static_assert(noexcept(ptr->WeakFromThis()), "Operation must be noexcept");
    static_assert(noexcept(cptr->WeakFromThis()), "Operation must be noexcept");

    WeakPtr<T> weak = ptr->WeakFromThis();
    REQUIRE(weak.Expired());

    WeakPtr<const T> my_const_weak = cptr->WeakFromThis();
    REQUIRE(my_const_weak.Expired());

    SharedPtr<T> sptr(ptr);
    weak = ptr->WeakFromThis();
    REQUIRE(!weak.Expired());
    REQUIRE(weak.Lock().Get() == ptr);
}
