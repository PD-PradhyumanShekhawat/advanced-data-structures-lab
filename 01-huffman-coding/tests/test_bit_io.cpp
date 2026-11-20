
#include "../include/bit_writer.hpp"
#include "../include/bit_reader.hpp"

#include <cassert>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

void test_full_byte() {
    std::stringstream stream;

    BitWriter writer(stream);
    const std::string input = "10110010";

    for (char bit : input) {
        writer.write_bit(bit == '1');
    }

    writer.finish();

    assert(stream.str().size() == 1);
    assert(static_cast<unsigned char>(stream.str()[0]) == 0xB2);

    BitReader reader(stream);
    std::string output;

    bool bit;
    while (reader.read_bit(bit)) {
        output += bit ? '1' : '0';
    }

    assert(output == input);
}

void test_partial_byte() {
    std::stringstream stream;

    BitWriter writer(stream);

    writer.write_bit(true);
    writer.write_bit(false);
    writer.write_bit(true);
    writer.finish();

    assert(static_cast<unsigned char>(stream.str()[0]) == 0xA0);

    BitReader reader(stream);

    bool bit;
    const bool expected[] = {true, false, true, false, false, false, false, false};

    for (bool expected_bit : expected) {
        assert(reader.read_bit(bit));
        assert(bit == expected_bit);
    }

    assert(!reader.read_bit(bit));
}

void test_empty_stream() {
    std::stringstream stream;
    BitReader reader(stream);

    bool bit = false;
    assert(!reader.read_bit(bit));
}

int main() {
    test_full_byte();
    test_partial_byte();
    test_empty_stream();

    std::cout << "All bit I/O tests passed.\n";
    return 0;
}
