//
// Created by Fanchao Liu on 15/07/20.
//

#ifndef GAMEMATCHER_FASTINTVECTOR_H
#define GAMEMATCHER_FASTINTVECTOR_H

#include <memory>
#include <cstring>

class FastIntVector {
    std::unique_ptr<int[]> data_;
    size_t capacity_ = 0, size_ = 0;

    void grow() {
        auto newCapacity = capacity_ == 0 ? 32 : capacity_ * 2;
        std::unique_ptr<int[]> data(new int[newCapacity]);
        if (data_) {
            ::memcpy(data.get(), data_.get(), capacity_ * sizeof(int));
        }

        capacity_ = newCapacity;
        data_ = std::move(data);
    }

public:
    inline auto size() const { return size_; }
    inline int& operator[](int index) { return *(data_.get() + index); }

    inline void push_back(int i) {
        if (size_ == capacity_) grow();
        (*this)[size_++] = i;
    }

    inline void pop_back() {
        if (size_ > 0) size_--;
    }
};

#endif //GAMEMATCHER_FASTINTVECTOR_H
