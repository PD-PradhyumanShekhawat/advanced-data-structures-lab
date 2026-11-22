
#include "../include/archive_format.hpp"
#include "../include/huffman_codec.hpp"

#include <cassert>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

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

    if (!data.empty()) {
        output.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size())
        );
    }

    if (!output) {
        throw std::runtime_error("Failed to write test file");
    }
}

void round_trip(
    const std::string& test_name,
    const std::vector<std::uint8_t>& original
) {
    const fs::path input = TEMP_DIR / (test_name + "_input.bin");
    const fs::path archive = TEMP_DIR / (test_name + ".huff");
    const fs::path restored = TEMP_DIR / (test_name + "_restored.bin");

    write_bytes(input, original);

    const auto stats = compress_file(input, archive);

    assert(stats.original_size == original.size());
    assert(fs::exists(archive));

    const auto restored_size = decompress_file(archive, restored);
    const auto decoded = read_bytes(restored);

    assert(restored_size == original.size());
    assert(decoded == original);

    fs::remove(input);
    fs::remove(archive);
    fs::remove(restored);
}

void expect_decode_failure(
    const fs::path& archive,
    const fs::path& output
) {
    bool rejected = false;

    try {
        decompress_file(archive, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_empty_file() {
    round_trip("huffman_v2_empty", {});
}

void test_single_symbol() {
    round_trip(
        "huffman_v2_single",
        std::vector<std::uint8_t>(1000, 'X')
    );
}

void test_text_file() {
    const std::string text =
        "Huffman coding is a greedy compression algorithm. "
        "Frequently occurring symbols receive shorter codes.";

    round_trip(
        "huffman_v2_text",
        std::vector<std::uint8_t>(text.begin(), text.end())
    );
}

void test_all_byte_values() {
    std::vector<std::uint8_t> data;

    for (int i = 0; i < 256; ++i) {
        data.push_back(static_cast<std::uint8_t>(i));
    }

    round_trip("huffman_v2_all_bytes", data);
}

void test_repeated_binary_data() {
    std::vector<std::uint8_t> data;

    for (int i = 0; i < 10000; ++i) {
        data.push_back(static_cast<std::uint8_t>(i % 7));
    }

    round_trip("huffman_v2_binary", data);
}

void test_compact_header() {
    const fs::path input = TEMP_DIR / "huffman_v2_compact_input.bin";
    const fs::path archive = TEMP_DIR / "huffman_v2_compact.huff";

    std::vector<std::uint8_t> data(1000, 'A');
    data.insert(data.end(), 500, 'B');
    data.insert(data.end(), 250, 'C');

    write_bytes(input, data);
    compress_file(input, archive);

    std::ifstream stream(archive, std::ios::binary);

    assert(stream);

    const auto header = ArchiveFormat::read_header(stream);
    const auto header_size = static_cast<std::uint64_t>(stream.tellg());

    assert(header.original_size == data.size());
    assert(header.frequencies['A'] == 1000);
    assert(header.frequencies['B'] == 500);
    assert(header.frequencies['C'] == 250);

    // Fixed metadata is 30 bytes.
    // Three frequency entries add 27 bytes.
    assert(header_size == 57);

    fs::remove(input);
    fs::remove(archive);
}

void test_corrupted_payload() {
    const fs::path input = TEMP_DIR / "huffman_v2_corrupt_input.bin";
    const fs::path archive = TEMP_DIR / "huffman_v2_corrupt.huff";
    const fs::path output = TEMP_DIR / "huffman_v2_corrupt_output.bin";

    const std::string text =
        "CRC32 should detect changes in reconstructed file contents.";

    const std::vector<std::uint8_t> data(text.begin(), text.end());

    write_bytes(input, data);
    compress_file(input, archive);

    auto archive_bytes = read_bytes(archive);
    assert(archive_bytes.size() > 57);

    // Flip a bit in the compressed payload.
    archive_bytes.back() ^= 0x80U;

    write_bytes(archive, archive_bytes);

    expect_decode_failure(archive, output);

    fs::remove(input);
    fs::remove(archive);

    if (fs::exists(output)) {
        fs::remove(output);
    }
}

void test_corrupted_checksum() {
    const fs::path input = TEMP_DIR / "huffman_v2_checksum_input.bin";
    const fs::path archive = TEMP_DIR / "huffman_v2_checksum.huff";
    const fs::path output = TEMP_DIR / "huffman_v2_checksum_output.bin";

    const std::string text = "Checksum validation test";
    const std::vector<std::uint8_t> data(text.begin(), text.end());

    write_bytes(input, data);
    compress_file(input, archive);

    auto archive_bytes = read_bytes(archive);

    // CRC-32 begins after:
    // 8-byte magic + 8-byte original size + 8-byte payload bit count.
    assert(archive_bytes.size() > 24);
    archive_bytes[24] ^= 0x01U;

    write_bytes(archive, archive_bytes);

    expect_decode_failure(archive, output);

    fs::remove(input);
    fs::remove(archive);

    if (fs::exists(output)) {
        fs::remove(output);
    }
}

void test_truncated_archive() {
    const fs::path input = TEMP_DIR / "huffman_v2_truncated_input.bin";
    const fs::path archive = TEMP_DIR / "huffman_v2_truncated.huff";
    const fs::path output = TEMP_DIR / "huffman_v2_truncated_output.bin";

    const std::string text =
        "This archive will be truncated to test validation.";

    const std::vector<std::uint8_t> data(text.begin(), text.end());

    write_bytes(input, data);
    compress_file(input, archive);

    auto archive_bytes = read_bytes(archive);

    assert(!archive_bytes.empty());

    archive_bytes.pop_back();
    write_bytes(archive, archive_bytes);

    expect_decode_failure(archive, output);

    fs::remove(input);
    fs::remove(archive);

    if (fs::exists(output)) {
        fs::remove(output);
    }
}

void test_trailing_archive_data() {
    const fs::path input = TEMP_DIR / "huffman_v2_trailing_input.bin";
    const fs::path archive = TEMP_DIR / "huffman_v2_trailing.huff";
    const fs::path output = TEMP_DIR / "huffman_v2_trailing_output.bin";

    const std::string text = "Trailing data must be rejected.";
    const std::vector<std::uint8_t> data(text.begin(), text.end());

    write_bytes(input, data);
    compress_file(input, archive);

    auto archive_bytes = read_bytes(archive);
    archive_bytes.push_back(0xFF);

    write_bytes(archive, archive_bytes);

    expect_decode_failure(archive, output);

    fs::remove(input);
    fs::remove(archive);

    if (fs::exists(output)) {
        fs::remove(output);
    }
}

void test_invalid_archive() {
    const fs::path archive = TEMP_DIR / "huffman_v2_invalid.huff";
    const fs::path output = TEMP_DIR / "huffman_v2_invalid_output.bin";

    write_bytes(archive, {1, 2, 3, 4, 5});

    expect_decode_failure(archive, output);

    fs::remove(archive);

    if (fs::exists(output)) {
        fs::remove(output);
    }
}

int main() {
    test_empty_file();
    test_single_symbol();
    test_text_file();
    test_all_byte_values();
    test_repeated_binary_data();
    test_compact_header();
    test_corrupted_payload();
    test_corrupted_checksum();
    test_truncated_archive();
    test_trailing_archive_data();
    test_invalid_archive();

    std::cout << "All Huffman v2 codec tests passed.\n";

    return 0;
}
