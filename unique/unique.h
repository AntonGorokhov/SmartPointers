#pragma once
#include <iostream>

#include "compressed_pair.h"

#include <cstddef>  // std::nullptr_t

template <typename T>
struct MyDeleter {
    void operator()(T* object) {
        delete object;
    }
};

template <typename T>
struct MyDeleter<T[]> {
    void operator()(T* object) {
        delete[] object;
    }
};

// Primary template
template <typename T, typename Deleter = MyDeleter<T>>
class UniquePtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        ptr_.GetFirst() = ptr;
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::move(deleter)) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        ptr_.GetFirst() = other.ptr_.GetFirst();
        ptr_.GetSecond() = std::move(other.ptr_.GetSecond());
        other.ptr_.GetFirst() = nullptr;
    }

    template <typename Y, typename Ydeleter = MyDeleter<Y>>
    UniquePtr(UniquePtr<Y, Ydeleter>&& other) noexcept {
        ptr_.GetFirst() = static_cast<T*>(other.Release());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset();
        ptr_.GetFirst() = other.ptr_.GetFirst();
        ptr_.GetSecond() = std::move(other.ptr_.GetSecond());
        other.ptr_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        if (ptr_.GetFirst()) {
            ptr_.GetSecond()(ptr_.GetFirst());
            ptr_.GetFirst() = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (ptr_.GetFirst()) {
            ptr_.GetSecond()(ptr_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr2 = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (ptr2) {
            ptr_.GetSecond()(ptr2);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> ptr_;
};

// Specialization for arrays
template <typename T, typename Deleter>
class UniquePtr<T[], Deleter> {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    explicit UniquePtr(T* ptr = nullptr) {
        ptr_.GetFirst() = ptr;
    }

    UniquePtr(T* ptr, Deleter&& deleter) : ptr_(ptr, std::move(deleter)) {
    }
    UniquePtr(T* ptr, const Deleter& deleter) : ptr_(ptr, deleter) {
    }

    UniquePtr(UniquePtr&& other) noexcept {
        ptr_.GetFirst() = other.ptr_.GetFirst();
        ptr_.GetSecond() = std::move(other.ptr_.GetSecond());
        other.ptr_.GetFirst() = nullptr;
    }

    template <typename Y, typename Ydeleter = MyDeleter<Y>>
    UniquePtr(UniquePtr<Y, Ydeleter>&& other) noexcept {
        ptr_.GetFirst() = static_cast<T*>(other.Release());
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    UniquePtr& operator=(UniquePtr&& other) noexcept {
        if (this == &other) {
            return *this;
        }
        Reset();
        ptr_.GetFirst() = other.ptr_.GetFirst();
        ptr_.GetSecond() = std::move(other.ptr_.GetSecond());
        other.ptr_.GetFirst() = nullptr;
        return *this;
    }
    UniquePtr& operator=(std::nullptr_t) {
        if (ptr_.GetFirst()) {
            ptr_.GetSecond()(ptr_.GetFirst());
            ptr_.GetFirst() = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~UniquePtr() {
        if (ptr_.GetFirst()) {
            ptr_.GetSecond()(ptr_.GetFirst());
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    T* Release() {
        auto ptr = ptr_.GetFirst();
        ptr_.GetFirst() = nullptr;
        return ptr;
    }
    void Reset(T* ptr = nullptr) {
        auto ptr2 = ptr_.GetFirst();
        ptr_.GetFirst() = ptr;
        if (ptr2) {
            ptr_.GetSecond()(ptr2);
        }
    }

    void Swap(UniquePtr& other) {
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_.GetFirst();
    }
    Deleter& GetDeleter() {
        return ptr_.GetSecond();
    }
    const Deleter& GetDeleter() const {
        return ptr_.GetSecond();
    }
    explicit operator bool() const {
        return ptr_.GetFirst() != nullptr;
    }

    T& operator[](size_t index) {
        return ptr_.GetFirst()[index];
    }

    const T& operator[](size_t index) const {
        return ptr_.GetFirst()[index];
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Single-object dereference operators

    typename std::add_lvalue_reference_t<T> operator*() const {
        return *ptr_.GetFirst();
    }
    T* operator->() const {
        return ptr_.GetFirst();
    }

private:
    CompressedPair<T*, Deleter> ptr_;
};
