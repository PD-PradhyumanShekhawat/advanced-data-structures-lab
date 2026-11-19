
#include "../include/frequency_table.hpp"
#include "../include/huffman_tree.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <memory>
#include <vector>

std::size_t count_leaves(
    const std::shared_ptr<HuffmanNode>& node
) {
    if (!node) {
        return 0;
    }

    if (node->is_leaf()) {
        return 1;
    }

    return count_leaves(node->left) +
           count_leaves(node->right);
}

void test_empty_input() {
    std::vector<std::uint8_t> data;

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);

    assert(root == nullptr);
}

void test_single_symbol() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'A', 'A'
    };

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);

    assert(root != nullptr);
    assert(root->is_leaf());
    assert(root->symbol == 'A');
    assert(root->frequency == 4);
}

void test_multiple_symbols() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'A', 'A', 'A',
        'B', 'B', 'B',
        'C', 'C',
        'D'
    };

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);

    assert(root != nullptr);
    assert(root->frequency == data.size());
    assert(count_leaves(root) == 4);
}

void test_frequency_analysis() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'B', 'C', 'C', 'C'
    };

    auto frequencies = calculate_frequencies(data);

    assert(frequencies['A'] == 2);
    assert(frequencies['B'] == 1);
    assert(frequencies['C'] == 3);
    assert(frequencies['D'] == 0);
}

int main() {
    test_empty_input();
    test_single_symbol();
    test_multiple_symbols();
    test_frequency_analysis();

    std::cout << "All Huffman tree tests passed.\n";

    return 0;
}
