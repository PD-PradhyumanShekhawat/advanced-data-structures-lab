
#pragma once

#include <cstddef>
#include <cstdint>

class CRC32 {
public:
    CRC32() = default;

    void update(const std::uint8_t* data, std::size_t size) {
        for (std::size_t i = 0; i < size; ++i) {
            state_ ^= data[i];

            for (unsigned bit = 0; bit < 8; ++bit) {
                if (state_ & 1U) {
                    state_ = (state_ >> 1) ^ 0xEDB88320U;
                } else {
                    state_ >>= 1;
                }
            }
        }
    }

    std::uint32_t value() const {
        return state_ ^ 0xFFFFFFFFU;
    }

private:
    std::uint32_t state_ = 0xFFFFFFFFU;
};

inline std::uint32_t calculate_crc32(
    const std::uint8_t* data,
    std::size_t size
) {
    CRC32 checksum;
    checksum.update(data, size);
    return checksum.value();
}

