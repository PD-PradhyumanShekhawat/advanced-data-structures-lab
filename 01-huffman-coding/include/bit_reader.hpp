
#pragma once

#include <cstdint>
#include <istream>
#include <limits>

class BitReader {
public:
    explicit BitReader(
        std::istream& input,
        std::uint64_t bit_limit = std::numeric_limits<std::uint64_t>::max()
    )
        : input_(input), bit_limit_(bit_limit) {}

    bool read_bit(bool& bit) {
        if (bits_read_ >= bit_limit_) {
            return false;
        }

        if (bits_remaining_ == 0) {
            const int next = input_.get();

            if (next == std::char_traits<char>::eof()) {
                return false;
            }

            buffer_ = static_cast<std::uint8_t>(next);
            bits_remaining_ = 8;
        }

        bit = (buffer_ & 0x80U) != 0;
        buffer_ <<= 1;

        --bits_remaining_;
        ++bits_read_;

        return true;
    }

    std::uint64_t bits_read() const noexcept {
        return bits_read_;
    }

    std::uint64_t bits_remaining_limit() const noexcept {
        return bit_limit_ - bits_read_;
    }

private:
    std::istream& input_;
    std::uint8_t buffer_ = 0;
    unsigned bits_remaining_ = 0;

    std::uint64_t bit_limit_;
    std::uint64_t bits_read_ = 0;
};
