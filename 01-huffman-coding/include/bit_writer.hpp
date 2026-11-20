
#pragma once

#include <cstdint>
#include <ostream>
#include <stdexcept>

class BitWriter {
public:
    explicit BitWriter(std::ostream& output)
        : output_(output) {}

    void write_bit(bool bit) {
        buffer_ = static_cast<std::uint8_t>(
            (buffer_ << 1) | (bit ? 1 : 0)
        );

        ++bits_filled_;

        if (bits_filled_ == 8) {
            flush_byte();
        }
    }

    void write_code(const std::string& code) {
        for (char bit : code) {
            if (bit != '0' && bit != '1') {
                throw std::invalid_argument("Invalid Huffman bit");
            }

            write_bit(bit == '1');
        }
    }

    void finish() {
        if (bits_filled_ > 0) {
            buffer_ <<= (8 - bits_filled_);
            flush_byte();
        }
    }

private:
    std::ostream& output_;
    std::uint8_t buffer_ = 0;
    unsigned bits_filled_ = 0;

    void flush_byte() {
        output_.put(static_cast<char>(buffer_));

        if (!output_) {
            throw std::runtime_error("Failed to write compressed data");
        }

        buffer_ = 0;
        bits_filled_ = 0;
    }
};
