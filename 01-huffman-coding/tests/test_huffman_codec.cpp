
#include "../include/huffman_codec.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

namespace fs = std::filesystem;

const fs::path TEMP_DIR = fs::temp_directory_path();

std::vector<std::uint8_t> read_bytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot read test file");
    }

    std::vector<std::uint8_t> result;
    char byte;

    while (input.get(byte)) {
        result.push_back(static_cast<std::uint8_t>(
            static_cast<unsigned char>(byte)
        ));
    }

    return result;
}

void write_bytes(
    const fs::path& path,
    const std::vector<std::uint8_t>& data
) {
    std::ofstream output(path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create test file");
    }

    for (auto byte : data) {
        output.put(static_cast<char>(byte));
    }
}

void round_trip(
    const std::string& test_name,
    const std::vector<std::uint8_t>& original
) {
    fs::path input = TEMP_DIR / (test_name + "_input.bin");
    fs::path archive = TEMP_DIR / (test_name + ".huff");
    fs::path restored = TEMP_DIR / (test_name + "_restored.bin");

    write_bytes(input, original);

    auto stats = compress_file(input, archive);

    assert(stats.original_size == original.size());
    assert(fs::exists(archive));

    auto restored_size = decompress_file(archive, restored);
    auto decoded = read_bytes(restored);

    assert(restored_size == original.size());
    assert(decoded == original);

    fs::remove(input);
    fs::remove(archive);
    fs::remove(restored);
}

void test_empty_file() {
    round_trip("huffman_empty", {});
}

void test_single_symbol() {
    round_trip("huffman_single", std::vector<std::uint8_t>(1000, 'X'));
}

void test_text_file() {
    std::string text =
        "Huffman coding is a greedy compression algorithm. "
        "Frequently occurring symbols receive shorter codes.";

    std::vector<std::uint8_t> data(text.begin(), text.end());

    round_trip("huffman_text", data);
}

void test_all_byte_values() {
    std::vector<std::uint8_t> data;

    for (int i = 0; i < 256; ++i) {
        data.push_back(static_cast<std::uint8_t>(i));
    }

    round_trip("huffman_all_bytes", data);
}

void test_invalid_archive() {
    fs::path archive = TEMP_DIR / "huffman_invalid.huff";
    fs::path output = TEMP_DIR / "huffman_invalid_output.bin";

    write_bytes(archive, {1, 2, 3, 4, 5});

    bool rejected = false;

    try {
        decompress_file(archive, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);

    fs::remove(archive);
    if (fs::exists(output)) fs::remove(output);
}

int main() {
    test_empty_file();
    test_single_symbol();
    test_text_file();
    test_all_byte_values();
    test_invalid_archive();

    std::cout << "All Huffman codec tests passed.\n";
    return 0;
}
