
#include "../include/huffman_codec.hpp"

#include "../include/archive_format.hpp"
#include "../include/bit_reader.hpp"
#include "../include/bit_writer.hpp"
#include "../include/crc32.hpp"
#include "../include/frequency_table.hpp"
#include "../include/huffman_codes.hpp"
#include "../include/huffman_tree.hpp"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

namespace {

std::vector<std::uint8_t> read_file(
    const std::filesystem::path& path
) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot open input file");
    }

    std::vector<std::uint8_t> data;
    char byte;

    while (input.get(byte)) {
        data.push_back(static_cast<std::uint8_t>(
            static_cast<unsigned char>(byte)
        ));
    }

    if (!input.eof()) {
        throw std::runtime_error("Failed while reading input file");
    }

    return data;
}

std::uint64_t calculate_payload_bits(
    const FrequencyTable& frequencies,
    const HuffmanCodeTable& codes
) {
    std::uint64_t total_bits = 0;

    for (std::size_t symbol = 0; symbol < frequencies.size(); ++symbol) {
        const std::uint64_t frequency = frequencies[symbol];

        if (frequency == 0) {
            continue;
        }

        const std::uint64_t code_length =
            static_cast<std::uint64_t>(codes[symbol].size());

        if (code_length == 0) {
            throw std::runtime_error("Missing Huffman code");
        }

        if (frequency >
            std::numeric_limits<std::uint64_t>::max() / code_length) {
            throw std::runtime_error("Encoded payload size overflow");
        }

        const std::uint64_t symbol_bits = frequency * code_length;

        if (symbol_bits >
            std::numeric_limits<std::uint64_t>::max() - total_bits) {
            throw std::runtime_error("Encoded payload size overflow");
        }

        total_bits += symbol_bits;
    }

    return total_bits;
}

void validate_payload_length(
    const std::filesystem::path& archive_path,
    std::istream& input,
    std::uint64_t payload_bits
) {
    const std::streampos payload_position = input.tellg();

    if (payload_position < 0) {
        throw std::runtime_error("Cannot determine payload position");
    }

    const std::uint64_t header_size =
        static_cast<std::uint64_t>(payload_position);

    const std::uint64_t payload_bytes =
        payload_bits / 8 + (payload_bits % 8 != 0 ? 1 : 0);

    if (payload_bytes >
        std::numeric_limits<std::uint64_t>::max() - header_size) {
        throw std::runtime_error("Archive size overflow");
    }

    const std::uint64_t expected_size = header_size + payload_bytes;
    const std::uint64_t actual_size =
        std::filesystem::file_size(archive_path);

    if (actual_size < expected_size) {
        throw std::runtime_error("Truncated compressed payload");
    }

    if (actual_size > expected_size) {
        throw std::runtime_error("Unexpected trailing archive data");
    }
}

} // namespace

CompressionStats compress_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path
) {
    const auto data = read_file(input_path);

    const auto frequencies = calculate_frequencies(data);
    const auto root = build_huffman_tree(frequencies);
    const auto codes = generate_huffman_codes(root);

    const std::uint64_t payload_bits =
        calculate_payload_bits(frequencies, codes);

    const std::uint32_t checksum = calculate_crc32(
        data.data(),
        data.size()
    );

    ArchiveHeader header;
    header.original_size = static_cast<std::uint64_t>(data.size());
    header.payload_bits = payload_bits;
    header.checksum = checksum;
    header.frequencies = frequencies;

    std::ofstream output(output_path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create output archive");
    }

    ArchiveFormat::write_header(output, header);

    BitWriter writer(output);

    for (std::uint8_t byte : data) {
        writer.write_code(codes[byte]);
    }

    writer.finish();

    if (writer.bits_written() != payload_bits) {
        throw std::runtime_error("Encoded bit count mismatch");
    }

    output.close();

    if (!output) {
        throw std::runtime_error("Failed to finalize archive");
    }

    const std::uint64_t archive_size =
        std::filesystem::file_size(output_path);

    const double ratio = data.empty()
        ? 0.0
        : static_cast<double>(archive_size) /
          static_cast<double>(data.size());

    return {
        static_cast<std::uint64_t>(data.size()),
        archive_size,
        ratio
    };
}

std::uint64_t decompress_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path
) {
    std::ifstream input(input_path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot open archive");
    }

    const ArchiveHeader header = ArchiveFormat::read_header(input);

    validate_payload_length(
        input_path,
        input,
        header.payload_bits
    );

    const auto root = build_huffman_tree(header.frequencies);
    const auto codes = generate_huffman_codes(root);

    const std::uint64_t expected_payload_bits =
        calculate_payload_bits(header.frequencies, codes);

    if (expected_payload_bits != header.payload_bits) {
        throw std::runtime_error(
            "Payload bit count does not match frequency table"
        );
    }

    if (header.original_size >
        std::vector<std::uint8_t>().max_size()) {
        throw std::runtime_error("Archive is too large to decode");
    }

    std::vector<std::uint8_t> decoded;
    decoded.reserve(static_cast<std::size_t>(header.original_size));

    if (header.original_size > 0 && !root) {
        throw std::runtime_error("Missing Huffman tree");
    }

    BitReader reader(input, header.payload_bits);

    if (root && root->is_leaf()) {
        for (std::uint64_t i = 0; i < header.original_size; ++i) {
            bool bit = false;

            if (!reader.read_bit(bit)) {
                throw std::runtime_error(
                    "Compressed payload ended unexpectedly"
                );
            }

            if (bit) {
                throw std::runtime_error(
                    "Invalid single-symbol Huffman payload"
                );
            }

            decoded.push_back(
                static_cast<std::uint8_t>(root->symbol)
            );
        }
    } else if (root) {
        while (decoded.size() < header.original_size) {
            auto node = root;

            while (!node->is_leaf()) {
                bool bit = false;

                if (!reader.read_bit(bit)) {
                    throw std::runtime_error(
                        "Compressed payload ended unexpectedly"
                    );
                }

                node = bit ? node->right : node->left;

                if (!node) {
                    throw std::runtime_error(
                        "Invalid Huffman bit sequence"
                    );
                }
            }

            decoded.push_back(
                static_cast<std::uint8_t>(node->symbol)
            );
        }
    }

    if (reader.bits_read() != header.payload_bits) {
        throw std::runtime_error(
            "Decoded data does not consume the declared payload bits"
        );
    }

    const auto decoded_frequencies = calculate_frequencies(decoded);

    if (decoded_frequencies != header.frequencies) {
        throw std::runtime_error(
            "Decoded data does not match archive frequencies"
        );
    }

    const std::uint32_t decoded_checksum = calculate_crc32(
        decoded.data(),
        decoded.size()
    );

    if (decoded_checksum != header.checksum) {
        throw std::runtime_error("CRC-32 checksum mismatch");
    }

    std::ofstream output(output_path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create decoded output file");
    }

    if (!decoded.empty()) {
        output.write(
            reinterpret_cast<const char*>(decoded.data()),
            static_cast<std::streamsize>(decoded.size())
        );
    }

    if (!output) {
        throw std::runtime_error("Failed to write decoded output");
    }

    output.close();

    if (!output) {
        throw std::runtime_error("Failed to finalize decoded output");
    }

    return header.original_size;
}
