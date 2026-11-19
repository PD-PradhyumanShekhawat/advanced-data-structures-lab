
#pragma once

#include "frequency_table.hpp"
#include "huffman_node.hpp"
#include "min_heap.hpp"

#include <cstdint>
#include <memory>

struct HuffmanNodeCompare {
    bool operator()(
        const std::shared_ptr<HuffmanNode>& a,
        const std::shared_ptr<HuffmanNode>& b
    ) const {

        if (a->frequency != b->frequency) {
            return a->frequency < b->frequency;
        }

        return a->min_symbol < b->min_symbol;
    }
};

inline std::shared_ptr<HuffmanNode> build_huffman_tree(
    const FrequencyTable& frequencies
) {

    MinHeap<
        std::shared_ptr<HuffmanNode>,
        HuffmanNodeCompare
    > heap;

    for (std::uint16_t symbol = 0; symbol < 256; ++symbol) {

        if (frequencies[symbol] == 0) {
            continue;
        }

        heap.push(
            std::make_shared<HuffmanNode>(
                frequencies[symbol],
                symbol
            )
        );
    }

    if (heap.empty()) {
        return nullptr;
    }

    while (heap.size() > 1) {

        auto left = heap.pop();
        auto right = heap.pop();

        auto parent = std::make_shared<HuffmanNode>(
            left->frequency + right->frequency,
            left,
            right
        );

        heap.push(parent);
    }

    return heap.pop();
}
