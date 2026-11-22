
#include "../include/bit_writer.hpp"
#include "../include/bit_reader.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>

void test_full_byte() {
    std::stringstream stream;

    BitWriter writer(stream);
    const std::string input = "10110010";

    for (char bit : input) {
        writer.write_bit(bit == '1');
    }

    writer.finish();

    assert(writer.bits_written() == 8);
    assert(stream.str().size() == 1);
    assert(static_cast<unsigned char>(stream.str()[0]) == 0xB2);

    BitReader reader(stream, 8);
    std::string output;

    bool bit = false;

    while (reader.read_bit(bit)) {
        output += bit ? '1' : '0';
    }

    assert(output == input);
    assert(reader.bits_read() == 8);
}

void test_partial_byte() {
    std::stringstream stream;

    BitWriter writer(stream);

    writer.write_bit(true);
    writer.write_bit(false);
    writer.write_bit(true);
    writer.finish();

    assert(writer.bits_written() == 3);
    assert(static_cast<unsigned char>(stream.str()[0]) == 0xA0);

    BitReader reader(stream, 3);

    bool bit = false;
    const bool expected[] = {true, false, true};

    for (bool expected_bit : expected) {
        assert(reader.read_bit(bit));
        assert(bit == expected_bit);
    }

    assert(!reader.read_bit(bit));
    assert(reader.bits_read() == 3);
}

void test_bounded_reader_ignores_padding() {
    std::stringstream stream;

    BitWriter writer(stream);

    writer.write_bit(true);
    writer.write_bit(false);
    writer.write_bit(true);
    writer.finish();

    BitReader reader(stream, 2);

    bool bit = false;

    assert(reader.read_bit(bit));
    assert(bit);

    assert(reader.read_bit(bit));
    assert(!bit);

    // The third encoded bit is not read because the limit is two.
    assert(!reader.read_bit(bit));
    assert(reader.bits_read() == 2);
}

void test_empty_stream() {
    std::stringstream stream;
    BitReader reader(stream);

    bool bit = false;

    assert(!reader.read_bit(bit));
    assert(reader.bits_read() == 0);
}

void test_truncated_stream() {
    std::stringstream stream;

    stream.put(static_cast<char>(0x80));
    stream.seekg(0);

    // Request 16 bits, but provide only one byte (8 bits).
    BitReader reader(stream, 16);

    bool bit = false;

    // First bit of 0x80 is 1.
    assert(reader.read_bit(bit));
    assert(bit);

    // Remaining seven bits are zero.
    for (int i = 0; i < 7; ++i) {
        assert(reader.read_bit(bit));
        assert(!bit);
    }

    // The stream contains no second byte.
    assert(!reader.read_bit(bit));
    assert(reader.bits_read() == 8);
}

int main() {
    test_full_byte();
    test_partial_byte();
    test_bounded_reader_ignores_padding();
    test_empty_stream();
    test_truncated_stream();

    std::cout << "All bit I/O tests passed.\n";

    return 0;
}
