#pragma once

class MyInt {
public:
    static int AliveCount() {
        return count_alive;
    }

    MyInt() {
        ++count_alive;
    }

    MyInt(int value) : value_(value) {
        ++count_alive;
    };

    MyInt(const MyInt& other) : value_(other.value_) {
        ++count_alive;
    }

    MyInt(MyInt&&) = delete;

    MyInt& operator=(const MyInt& other) {
        value_ = other.value_;
        ++count_alive;
        return *this;
    }

    MyInt& operator=(MyInt&& other) = delete;

    ~MyInt() {
        --count_alive;
    }

    bool operator==(int other) const {
        return value_ == other;
    }

private:
    int value_;

    inline static int count_alive = 0;
};
