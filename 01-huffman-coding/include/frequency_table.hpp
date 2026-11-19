
#pragma once

#include <array>
#include <cstdint>
#include <vector>

using FrequencyTable = std::array<std::uint64_t, 256>;

inline FrequencyTable calculate_frequencies(
    const std::vector<std::uint8_t>& data) {

    FrequencyTable frequencies{};

    for (std::uint8_t byte : data) {
        ++frequencies[byte];
    }

    return frequencies;
}
