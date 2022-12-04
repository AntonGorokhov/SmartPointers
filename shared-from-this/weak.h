#pragma once

#include "sw_fwd.h"  // Forward declaration
#include "shared.h"

// https://en.cppreference.com/w/cpp/memory/weak_ptr
template <typename T>
class WeakPtr {
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Constructors

    WeakPtr() {
    }

    WeakPtr(const WeakPtr& other) {
        if (this != &other) {
            block_ = other.block_;
            ptr_ = other.ptr_;
            if (block_) {
                ++block_->cnt_weak;
            }
        }
    }

    template <class Y>
    WeakPtr(const WeakPtr<Y>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt_weak;
        }
    }

    WeakPtr(WeakPtr&& other) {
        if (this != &other) {
            block_ = other.block_;
            ptr_ = other.ptr_;
            other.block_ = nullptr;
            other.ptr_ = nullptr;
        }
    }

    // Demote `SharedPtr`
    // #2 from https://en.cppreference.com/w/cpp/memory/weak_ptr/weak_ptr
    WeakPtr(const SharedPtr<T>& other) {
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt_weak;
        }
    }

    WeakPtr& operator=(const SharedPtr<T>& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt_weak;
        }
        return *this;
    }

    template <class Y>
    WeakPtr& operator=(const SharedPtr<Y>& other) {
        Reset();
        block_ = other.block_;
        ptr_ = other.ptr_;
        if (block_) {
            ++block_->cnt_weak;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // `operator=`-s

    WeakPtr& operator=(const WeakPtr& other) {
        if (this != &other) {
            Reset();
            block_ = other.block_;
            ptr_ = other.ptr_;
            if (block_) {
                ++block_->cnt_weak;
            }
        }
        return *this;
    }
    WeakPtr& operator=(WeakPtr&& other) {
        if (this != &other) {
            Reset();
            block_ = other.block_;
            ptr_ = other.ptr_;
            other.block_ = nullptr;
            other.ptr_ = nullptr;
        }
        return *this;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Destructor

    ~WeakPtr() {
        Reset();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Modifiers

    void Reset() {
        if (block_) {
            --block_->cnt_weak;
            if (block_->cnt_weak == 0 && block_->cnt == 0) {
                delete block_;
            }
        }
        block_ = nullptr;
        ptr_ = nullptr;
    }
    void Swap(WeakPtr& other) {
        std::swap(block_, other.block_);
        std::swap(ptr_, other.ptr_);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////
    // Observers

    size_t UseCount() const {
        if (!block_) {
            return 0;
        }
        return block_->cnt;
    }
    bool Expired() const {
        if (ptr_ == nullptr) {
            return true;
        }
        if (block_ == nullptr) {
            return true;
        }
        if (block_->cnt == 0) {
            return true;
        }
        return false;
    }
    SharedPtr<T> Lock() const {
        if (block_) {
            return SharedPtr<T>(block_, ptr_, true);
        }
        return SharedPtr<T>(nullptr);
    }
    BlockBase* block_ = nullptr;
    T* ptr_ = nullptr;
};
