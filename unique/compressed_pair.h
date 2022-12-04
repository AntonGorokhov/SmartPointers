#pragma once

#include <type_traits>
#include <utility>

// Me think, why waste time write lot code, when few code do trick.

template <typename T, std::size_t I, bool = std::is_empty_v<T> && !std::is_final_v<T>>
struct CompressedPairElement {
    T value;
    CompressedPairElement() = default;
    CompressedPairElement(const T& val) : value(val) {
    }
    CompressedPairElement(T&& val) : value(std::move(val)) {
    }
    T& GetElement() {
        return value;
    }
    const T& GetElement() const {
        return value;
    }
};

template <typename T, std::size_t I>
struct CompressedPairElement<T, I, true> : public T {
    CompressedPairElement(const T& value) {
    }
    CompressedPairElement() = default;
    CompressedPairElement(T&& val) {
    }
    T& GetElement() {
        return *this;
    }
    const T& GetElement() const {
        return *this;
    }
};

template <typename F, typename S>
class CompressedPair : private CompressedPairElement<F, 0>, private CompressedPairElement<S, 1> {
    using First = CompressedPairElement<F, 0>;
    using Second = CompressedPairElement<S, 1>;

public:
    CompressedPair(const F& first, const S& second) : First(first), Second(second) {
    }
    CompressedPair(F&& first, const S& second) : First(std::move(first)), Second(second) {
    }
    CompressedPair(F&& first, S&& second) : First(std::move(first)), Second(std::move(second)) {
    }
    CompressedPair(const F& first, S&& second) : First(first), Second(std::move(second)) {
    }

    CompressedPair() = default;

    const F& GetFirst() const {
        return First::GetElement();
    }

    const S& GetSecond() const {
        return Second::GetElement();
    };

    F& GetFirst() {
        return First::GetElement();
    }

    S& GetSecond() {
        return Second::GetElement();
    };
};