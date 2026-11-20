
#include "../include/frequency_table.hpp"
#include "../include/huffman_tree.hpp"
#include "../include/huffman_codes.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

void test_code_generation() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'A', 'A', 'A',
        'B', 'B', 'B', 'C', 'C', 'D'
    };

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_huffman_codes(root);

    for (std::size_t i = 0; i < 256; ++i) {
        if (frequencies[i] > 0) {
            assert(!codes[i].empty());
        }
    }
}

void test_prefix_free_property() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'A', 'B', 'B', 'C', 'D'
    };

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_huffman_codes(root);

    for (std::size_t i = 0; i < 256; ++i) {
        if (frequencies[i] == 0) continue;

        for (std::size_t j = 0; j < 256; ++j) {
            if (i == j || frequencies[j] == 0) continue;

            assert(codes[j].compare(
                0, codes[i].size(), codes[i]
            ) != 0);
        }
    }
}

void test_single_symbol() {
    std::vector<std::uint8_t> data(10, 'X');

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_huffman_codes(root);

    assert(codes['X'] == "0");
    assert(calculate_wepl(frequencies, codes) == 10);
}

void test_wepl() {
    std::vector<std::uint8_t> data = {
        'A', 'A', 'A', 'A',
        'B', 'B',
        'C', 'C',
        'D'
    };

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_huffman_codes(root);

    std::uint64_t wepl = calculate_wepl(frequencies, codes);

    std::uint64_t expected = 0;

    for (std::size_t i = 0; i < 256; ++i) {
        expected += frequencies[i] * codes[i].size();
    }

    assert(wepl == expected);
}

int main() {
    test_code_generation();
    test_prefix_free_property();
    test_single_symbol();
    test_wepl();

    std::cout << "All Huffman code tests passed.\n";
    return 0;
}

