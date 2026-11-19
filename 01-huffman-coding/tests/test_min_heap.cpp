
#include "../include/min_heap.hpp"

#include <cassert>
#include <iostream>
#include <stdexcept>

struct MinCompare {
    bool operator()(int a, int b) const {
        return a < b;
    }
};

void test_empty_heap() {
    MinHeap<int, MinCompare> heap;

    assert(heap.empty());
    assert(heap.size() == 0);
}

void test_single_element() {
    MinHeap<int, MinCompare> heap;

    heap.push(42);

    assert(!heap.empty());
    assert(heap.size() == 1);
    assert(heap.top() == 42);
    assert(heap.pop() == 42);
    assert(heap.empty());
}

void test_minimum_ordering() {
    MinHeap<int, MinCompare> heap;

    heap.push(30);
    heap.push(10);
    heap.push(50);
    heap.push(20);
    heap.push(5);

    assert(heap.top() == 5);

    assert(heap.pop() == 5);
    assert(heap.pop() == 10);
    assert(heap.pop() == 20);
    assert(heap.pop() == 30);
    assert(heap.pop() == 50);

    assert(heap.empty());
}

void test_duplicate_values() {
    MinHeap<int, MinCompare> heap;

    heap.push(10);
    heap.push(5);
    heap.push(10);
    heap.push(5);

    assert(heap.pop() == 5);
    assert(heap.pop() == 5);
    assert(heap.pop() == 10);
    assert(heap.pop() == 10);
}

void test_empty_operations() {
    MinHeap<int, MinCompare> heap;

    bool top_threw = false;
    bool pop_threw = false;

    try {
        heap.top();
    } catch (const std::out_of_range&) {
        top_threw = true;
    }

    try {
        heap.pop();
    } catch (const std::out_of_range&) {
        pop_threw = true;
    }

    assert(top_threw);
    assert(pop_threw);
}

int main() {
    test_empty_heap();
    test_single_element();
    test_minimum_ordering();
    test_duplicate_values();
    test_empty_operations();

    std::cout << "All MinHeap tests passed.\n";

    return 0;
}
