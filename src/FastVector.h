//
// Created by Fanchao Liu on 15/07/20.
//

#ifndef GAMEMATCHER_FASTVECTOR_H
#define GAMEMATCHER_FASTVECTOR_H

#include <memory>
#include <cstring>
#include <type_traits>

template <typename T>
class FastVector {
    std::unique_ptr<T[]> data_;
    size_t capacity_ = 0, size_ = 0;

    void grow() {
        auto newCapacity = capacity_ == 0 ? 32 : capacity_ * 2;
        auto data = std::make_unique<T[]>(newCapacity);
        if (data_) {
            ::memcpy(data.get(), data_.get(), capacity_ * sizeof(T));
        }

        capacity_ = newCapacity;
        data_ = std::move(data);
    }

public:
    inline auto size() const { return size_; }
    inline T& operator[](int index) { return *(data_.get() + index); }

    inline bool empty() const {
        return size() == 0;
    }

    inline void push_back(T i) {
        if (size_ == capacity_) grow();
        (*this)[size_++] = i;
    }

    inline void pop_back() {
        if (size_ > 0) size_--;
    }
};

#endif //GAMEMATCHER_FASTVECTOR_H
