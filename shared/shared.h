#pragma once

#include "sw_fwd.h"  // Forward declaration

#include <cstddef>  // std::nullptr_t
#include <iostream>

struct BlockBase {
    size_t cnt{1};
    virtual ~BlockBase() = default;
};

template <typename T>
class Block : public BlockBase {
public:
    explicit Block(T* ptr) : ptr(ptr) {
    }
    ~Block() override {
        delete ptr;
    }
    T* Get() {
        return ptr;
    }
    T* ptr;
};

template <typename T>
class BlockEmplace : public BlockBase {
public:
    template <typename... Args>
    BlockEmplace(Args&&... args) {
        new (storage) T{std::forward<Args>(args)...};
    }
    ~BlockEmplace() override {
        //        std::destroy_at(std::launder(reinterpret_cast<T*>(&storage)));
        reinterpret_cast<T*>(&storage)->~T();
        //        delete elem;
    }
    T* Get() {
        return reinterpret_cast<T*>(&storage);
        //        return elem;
    }
    //    std::aligned_storage<sizeof(T), alignof(T)> storage;
    alignas(T) char storage[sizeof(T)];
    //    T* elem;
};

// https://en.cppreference.com/w/cpp/memory/shared_ptr
template <typename T>
class SharedPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors
    T* ptr_ = nullptr;
    SharedPtr() {
        block_ = nullptr;
    }
    SharedPtr(std::nullptr_t) {
        block_ = nullptr;
    }
    explicit SharedPtr(T* ptr) {
        // std::cout<< "Constructing block\n";
        block_ = new Block(ptr);
        ptr_ = ptr;
    }

    template <typename Y>
    explicit SharedPtr(Y* ptr) {
        // std::cout<< "Constructing block\n";
        block_ = new Block<Y>(ptr);
        ptr_ = ptr;
    }

    explicit SharedPtr(BlockBase* bb, T* ptr) {
        // std::cout<< "Constructing block from makes shared\n";
        block_ = bb;
        ptr_ = ptr;
    }

    SharedPtr(const SharedPtr& other) {
        // std::cout<< "Constructing from shared\n";
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt;
        }
    }

    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other) {
        // std::cout<< "Constructing from shared\n";
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt;
        }
    }

    /*SharedPtr(SharedPtr<T>&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }*/

    template <typename Y>
    SharedPtr<T>(SharedPtr<Y>&& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
    }

    // Aliasing constructor
    // #8 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    template <typename Y>
    SharedPtr(const SharedPtr<Y>& other, T* ptr) {
        ptr_ = ptr;
        block_ = other.block_;
        if (block_) {
            ++block_->cnt;
        }
    }

    // Promote `WeakPtr`
    // #11 from https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr
    //    explicit SharedPtr(const WeakPtr<T>& other);

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    SharedPtr& operator=(const SharedPtr& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt;
        }
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(const SharedPtr<Y>& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt;
        }
        return *this;
    }

    SharedPtr& operator=(SharedPtr&& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    template <typename Y>
    SharedPtr& operator=(SharedPtr<Y>&& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        other.block_ = nullptr;
        other.ptr_ = nullptr;
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~SharedPtr() {
        if (block_) {
            --block_->cnt;
            if (block_->cnt == 0) {
                delete block_;
            }
        }
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        auto prev = block_;
        block_ = nullptr;
        if (prev) {
            --prev->cnt;
            if (prev->cnt == 0) {
                delete prev;
            }
        }
        ptr_ = nullptr;
    }
    void Reset(T* ptr) {
        auto prev = block_;
        block_ = nullptr;
        if (prev) {
            --prev->cnt;
            if (prev->cnt == 0) {
                delete prev;
            }
        }
        block_ = new Block(ptr);
        ptr_ = ptr;
    }
    template <typename Y>
    void Reset(Y* ptr) {
        auto prev = block_;
        block_ = nullptr;
        if (prev) {
            --prev->cnt;
            if (prev->cnt == 0) {
                delete prev;
            }
        }
        block_ = new Block<Y>(ptr);
        ptr_ = ptr;
    }
    void Swap(SharedPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    T* Get() const {
        return ptr_;
    }
    T& operator*() const {
        return *ptr_;
    }
    T* operator->() const {
        return Get();
    }
    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->cnt;
    }
    explicit operator bool() const {
        return block_;
    }
    BlockBase* block_ = nullptr;
};

template <typename T, typename U>
inline bool operator==(const SharedPtr<T>& left, const SharedPtr<U>& right);

// Allocate memory only once
template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args);

// Look for usage examples in tests
template <typename T>
class EnableSharedFromThis {
public:
    SharedPtr<T> SharedFromThis();
    SharedPtr<const T> SharedFromThis() const;

    WeakPtr<T> WeakFromThis() noexcept;
    WeakPtr<const T> WeakFromThis() const noexcept;
};

template <typename T, typename... Args>
SharedPtr<T> MakeShared(Args&&... args) {
    // std::cout<< "Makes shared\n";
    auto block = new BlockEmplace<T>(std::forward<Args>(args)...);
    return SharedPtr<T>(block, block->Get());
}