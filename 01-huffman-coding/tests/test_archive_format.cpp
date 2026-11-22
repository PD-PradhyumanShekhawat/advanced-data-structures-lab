
#include "../include/archive_format.hpp"

#include <cassert>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <stdexcept>

void test_header_round_trip() {
    ArchiveHeader original;

    original.original_size = 100;
    original.payload_bits = 175;
    original.checksum = 0x12345678U;

    original.frequencies['A'] = 60;
    original.frequencies['B'] = 30;
    original.frequencies['C'] = 10;

    std::stringstream stream(
        std::ios::in | std::ios::out | std::ios::binary
    );

    ArchiveFormat::write_header(stream, original);

    stream.seekg(0);

    auto restored = ArchiveFormat::read_header(stream);

    assert(restored.original_size == original.original_size);
    assert(restored.payload_bits == original.payload_bits);
    assert(restored.checksum == original.checksum);

    assert(restored.frequencies['A'] == 60);
    assert(restored.frequencies['B'] == 30);
    assert(restored.frequencies['C'] == 10);
    assert(restored.frequencies['D'] == 0);
}

void test_empty_archive_header() {
    ArchiveHeader original;

    std::stringstream stream(
        std::ios::in | std::ios::out | std::ios::binary
    );

    ArchiveFormat::write_header(stream, original);

    stream.seekg(0);

    auto restored = ArchiveFormat::read_header(stream);

    assert(restored.original_size == 0);
    assert(restored.payload_bits == 0);
}

void test_invalid_magic() {
    std::stringstream stream(
        std::string("INVALID!"),
        std::ios::in | std::ios::binary
    );

    bool rejected = false;

    try {
        ArchiveFormat::read_header(stream);
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    assert(rejected);
}

void test_truncated_header() {
    std::stringstream stream(
        std::string("HUFF"),
        std::ios::in | std::ios::binary
    );

    bool rejected = false;

    try {
        ArchiveFormat::read_header(stream);
    } catch (const std::runtime_error&) {
        rejected = true;
    }

    assert(rejected);
}

int main() {
    test_header_round_trip();
    test_empty_archive_header();
    test_invalid_magic();
    test_truncated_header();

    std::cout << "All archive format tests passed.\n";

    return 0;
}
