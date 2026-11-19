
#pragma once

#include <cstdint>
#include <memory>

struct HuffmanNode {
    std::uint64_t frequency;
    std::uint16_t symbol;
    std::uint16_t min_symbol;

    std::shared_ptr<HuffmanNode> left;
    std::shared_ptr<HuffmanNode> right;

    HuffmanNode(
        std::uint64_t frequency,
        std::uint16_t symbol
    )
        : frequency(frequency),
          symbol(symbol),
          min_symbol(symbol),
          left(nullptr),
          right(nullptr) {}

    HuffmanNode(
        std::uint64_t frequency,
        std::shared_ptr<HuffmanNode> left,
        std::shared_ptr<HuffmanNode> right
    )
        : frequency(frequency),
          symbol(256),
          min_symbol(
              left->min_symbol < right->min_symbol
                  ? left->min_symbol
                  : right->min_symbol
          ),
          left(std::move(left)),
          right(std::move(right)) {}

    bool is_leaf() const {
        return left == nullptr && right == nullptr;
    }
};
