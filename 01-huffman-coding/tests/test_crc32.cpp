
#include "../include/crc32.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

void test_standard_crc32_vector() {
    const std::string input = "123456789";

    const auto checksum = calculate_crc32(
        reinterpret_cast<const std::uint8_t*>(input.data()),
        input.size()
    );

    assert(checksum == 0xCBF43926U);
}

void test_empty_input() {
    const auto checksum = calculate_crc32(nullptr, 0);

    assert(checksum == 0x00000000U);
}

void test_incremental_checksum() {
    const std::string first = "1234";
    const std::string second = "56789";

    CRC32 checksum;

    checksum.update(
        reinterpret_cast<const std::uint8_t*>(first.data()),
        first.size()
    );

    checksum.update(
        reinterpret_cast<const std::uint8_t*>(second.data()),
        second.size()
    );

    assert(checksum.value() == 0xCBF43926U);
}

void test_different_inputs() {
    const std::string first = "Huffman";
    const std::string second = "Huffmant";

    const auto crc_first = calculate_crc32(
        reinterpret_cast<const std::uint8_t*>(first.data()),
        first.size()
    );

    const auto crc_second = calculate_crc32(
        reinterpret_cast<const std::uint8_t*>(second.data()),
        second.size()
    );

    assert(crc_first != crc_second);
}

int main() {
    test_standard_crc32_vector();
    test_empty_input();
    test_incremental_checksum();
    test_different_inputs();

    std::cout << "All CRC-32 tests passed.\n";

    return 0;
}
