
#pragma once

#include "frequency_table.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>

struct ArchiveHeader {
    std::uint64_t original_size = 0;
    std::uint64_t payload_bits = 0;
    std::uint32_t checksum = 0;
    FrequencyTable frequencies{};
};

namespace ArchiveFormat {

constexpr std::array<char, 8> MAGIC = {
    'H', 'U', 'F', 'F', 'M', 'A', 'N', '2'
};

inline void write_u16(std::ostream& out, std::uint16_t value) {
    for (unsigned i = 0; i < 2; ++i) {
        out.put(static_cast<char>(value & 0xFF));
        value >>= 8;
    }

    if (!out) {
        throw std::runtime_error("Failed to write archive header");
    }
}

inline void write_u32(std::ostream& out, std::uint32_t value) {
    for (unsigned i = 0; i < 4; ++i) {
        out.put(static_cast<char>(value & 0xFF));
        value >>= 8;
    }

    if (!out) {
        throw std::runtime_error("Failed to write archive header");
    }
}

inline void write_u64(std::ostream& out, std::uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) {
        out.put(static_cast<char>(value & 0xFF));
        value >>= 8;
    }

    if (!out) {
        throw std::runtime_error("Failed to write archive header");
    }
}

inline std::uint16_t read_u16(std::istream& in) {
    std::uint16_t value = 0;

    for (unsigned i = 0; i < 2; ++i) {
        int byte = in.get();

        if (byte == std::char_traits<char>::eof()) {
            throw std::runtime_error("Truncated archive header");
        }

        value |= static_cast<std::uint16_t>(
            static_cast<unsigned char>(byte)
        ) << (i * 8);
    }

    return value;
}

inline std::uint32_t read_u32(std::istream& in) {
    std::uint32_t value = 0;

    for (unsigned i = 0; i < 4; ++i) {
        int byte = in.get();

        if (byte == std::char_traits<char>::eof()) {
            throw std::runtime_error("Truncated archive header");
        }

        value |= static_cast<std::uint32_t>(
            static_cast<unsigned char>(byte)
        ) << (i * 8);
    }

    return value;
}

inline std::uint64_t read_u64(std::istream& in) {
    std::uint64_t value = 0;

    for (unsigned i = 0; i < 8; ++i) {
        int byte = in.get();

        if (byte == std::char_traits<char>::eof()) {
            throw std::runtime_error("Truncated archive header");
        }

        value |= static_cast<std::uint64_t>(
            static_cast<unsigned char>(byte)
        ) << (i * 8);
    }

    return value;
}

inline void write_header(
    std::ostream& out,
    const ArchiveHeader& header
) {
    std::uint16_t symbol_count = 0;

    for (auto frequency : header.frequencies) {
        if (frequency > 0) {
            ++symbol_count;
        }
    }

    out.write(MAGIC.data(), MAGIC.size());

    if (!out) {
        throw std::runtime_error("Failed to write archive magic");
    }

    write_u64(out, header.original_size);
    write_u64(out, header.payload_bits);
    write_u32(out, header.checksum);
    write_u16(out, symbol_count);

    for (std::size_t symbol = 0;
         symbol < header.frequencies.size();
         ++symbol) {

        const auto frequency = header.frequencies[symbol];

        if (frequency == 0) {
            continue;
        }

        out.put(static_cast<char>(symbol));
        write_u64(out, frequency);
    }
}

inline ArchiveHeader read_header(std::istream& in) {
    std::array<char, 8> magic{};

    in.read(magic.data(), magic.size());

    if (!in || magic != MAGIC) {
        throw std::runtime_error("Invalid Huffman v2 archive");
    }

    ArchiveHeader header;

    header.original_size = read_u64(in);
    header.payload_bits = read_u64(in);
    header.checksum = read_u32(in);

    const std::uint16_t symbol_count = read_u16(in);

    if (symbol_count > 256) {
        throw std::runtime_error("Invalid symbol count");
    }

    std::uint64_t total_frequency = 0;

    for (std::uint16_t i = 0; i < symbol_count; ++i) {
        int symbol = in.get();

        if (symbol == std::char_traits<char>::eof()) {
            throw std::runtime_error("Truncated frequency table");
        }

        const auto index = static_cast<unsigned char>(symbol);

        if (header.frequencies[index] != 0) {
            throw std::runtime_error("Duplicate symbol in frequency table");
        }

        const auto frequency = read_u64(in);

        if (frequency == 0) {
            throw std::runtime_error("Invalid zero symbol frequency");
        }

        if (frequency >
            UINT64_MAX - total_frequency) {
            throw std::runtime_error("Frequency table overflow");
        }

        header.frequencies[index] = frequency;
        total_frequency += frequency;
    }

    if (total_frequency != header.original_size) {
        throw std::runtime_error(
            "Frequency table does not match original size"
        );
    }

    if ((header.original_size == 0) != (symbol_count == 0)) {
        throw std::runtime_error("Invalid empty archive metadata");
    }

    if ((header.original_size == 0) != (header.payload_bits == 0)) {
        throw std::runtime_error("Invalid empty archive payload size");
    }

    return header;
}

} // namespace ArchiveFormat
