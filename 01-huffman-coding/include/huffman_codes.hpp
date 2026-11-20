
#pragma once

#include "frequency_table.hpp"
#include "huffman_node.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>

using HuffmanCodeTable = std::array<std::string, 256>;

inline void generate_codes_recursive(
    const std::shared_ptr<HuffmanNode>& node,
    const std::string& prefix,
    HuffmanCodeTable& codes
) {
    if (!node) return;

    if (node->is_leaf()) {
        codes[node->symbol] = prefix.empty() ? "0" : prefix;
        return;
    }

    generate_codes_recursive(node->left, prefix + "0", codes);
    generate_codes_recursive(node->right, prefix + "1", codes);
}

inline HuffmanCodeTable generate_huffman_codes(
    const std::shared_ptr<HuffmanNode>& root
) {
    HuffmanCodeTable codes{};
    generate_codes_recursive(root, "", codes);
    return codes;
}

inline std::uint64_t calculate_wepl(
    const FrequencyTable& frequencies,
    const HuffmanCodeTable& codes
) {
    std::uint64_t wepl = 0;

    for (std::size_t symbol = 0; symbol < 256; ++symbol) {
        if (frequencies[symbol] == 0) continue;

        wepl += frequencies[symbol] * codes[symbol].size();
    }

    return wepl;
}
