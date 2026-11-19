
#pragma once

#include <cstddef>
#include <stdexcept>
#include <utility>
#include <vector>

template <typename T, typename Compare>
class MinHeap {
public:
    explicit MinHeap(Compare compare = Compare{})
        : compare_(std::move(compare)) {}

    bool empty() const noexcept {
        return data_.empty();
    }

    std::size_t size() const noexcept {
        return data_.size();
    }

    const T& top() const {
        if (empty()) {
            throw std::out_of_range("MinHeap is empty");
        }

        return data_.front();
    }

    void push(T value) {
        data_.push_back(std::move(value));
        sift_up(data_.size() - 1);
    }

    T pop() {
        if (empty()) {
            throw std::out_of_range("Cannot pop from an empty MinHeap");
        }

        T minimum = std::move(data_.front());

        if (data_.size() == 1) {
            data_.pop_back();
            return minimum;
        }

        data_.front() = std::move(data_.back());
        data_.pop_back();

        sift_down(0);

        return minimum;
    }

private:
    std::vector<T> data_;
    Compare compare_;

    void sift_up(std::size_t index) {
        while (index > 0) {
            std::size_t parent = (index - 1) / 2;

            if (!compare_(data_[index], data_[parent])) {
                break;
            }

            std::swap(data_[index], data_[parent]);
            index = parent;
        }
    }

    void sift_down(std::size_t index) {
        while (true) {
            std::size_t left = 2 * index + 1;
            std::size_t right = 2 * index + 2;
            std::size_t smallest = index;

            if (left < data_.size() &&
                compare_(data_[left], data_[smallest])) {
                smallest = left;
            }

            if (right < data_.size() &&
                compare_(data_[right], data_[smallest])) {
                smallest = right;
            }

            if (smallest == index) {
                break;
            }

            std::swap(data_[index], data_[smallest]);
            index = smallest;
        }
    }
};
