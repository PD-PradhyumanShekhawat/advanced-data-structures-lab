
#pragma once

#include <cstdint>
#include <istream>

class BitReader {
public:
    explicit BitReader(std::istream& input)
        : input_(input) {}

    bool read_bit(bool& bit) {
        if (bits_remaining_ == 0) {
            int next = input_.get();

            if (next == std::char_traits<char>::eof()) {
                return false;
            }

            buffer_ = static_cast<std::uint8_t>(next);
            bits_remaining_ = 8;
        }

        bit = (buffer_ & 0x80) != 0;
        buffer_ <<= 1;
        --bits_remaining_;

        return true;
    }

private:
    std::istream& input_;
    std::uint8_t buffer_ = 0;
    unsigned bits_remaining_ = 0;
};
