
#include "../include/huffman_codec.hpp"

#include "../include/bit_reader.hpp"
#include "../include/bit_writer.hpp"
#include "../include/frequency_table.hpp"
#include "../include/huffman_codes.hpp"
#include "../include/huffman_tree.hpp"

#include <array>
#include <fstream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

constexpr char MAGIC[8] = {
    'H', 'U', 'F', 'F', 'M', 'A', 'N', '1'
};

void write_u64(std::ostream& out, std::uint64_t value) {
    for (unsigned i = 0; i < 8; ++i) {
        out.put(static_cast<char>(value & 0xFF));
        value >>= 8;
    }

    if (!out) {
        throw std::runtime_error("Failed to write archive header");
    }
}

std::uint64_t read_u64(std::istream& in) {
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

void write_header(
    std::ostream& output,
    std::uint64_t original_size,
    const FrequencyTable& frequencies
) {
    output.write(MAGIC, sizeof(MAGIC));
    write_u64(output, original_size);

    for (std::uint64_t frequency : frequencies) {
        write_u64(output, frequency);
    }
}

FrequencyTable read_header(
    std::istream& input,
    std::uint64_t& original_size
) {
    char magic[sizeof(MAGIC)];

    input.read(magic, sizeof(magic));

    if (!input || !std::equal(
            std::begin(magic), std::end(magic),
            std::begin(MAGIC))) {
        throw std::runtime_error("Invalid Huffman archive");
    }

    original_size = read_u64(input);

    FrequencyTable frequencies{};

    std::uint64_t total = 0;

    for (std::size_t i = 0; i < frequencies.size(); ++i) {
        frequencies[i] = read_u64(input);

        if (frequencies[i] >
            std::numeric_limits<std::uint64_t>::max() - total) {
            throw std::runtime_error("Frequency table overflow");
        }

        total += frequencies[i];
    }

    if (total != original_size) {
        throw std::runtime_error(
            "Archive size does not match frequency table"
        );
    }

    return frequencies;
}

} // namespace

CompressionStats compress_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path
) {
    auto data = read_file(input_path);

    auto frequencies = calculate_frequencies(data);
    auto root = build_huffman_tree(frequencies);
    auto codes = generate_huffman_codes(root);

    std::ofstream output(output_path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create output archive");
    }

    write_header(output, data.size(), frequencies);

    BitWriter writer(output);

    for (std::uint8_t byte : data) {
        writer.write_code(codes[byte]);
    }

    writer.finish();
    output.close();

    if (!output) {
        throw std::runtime_error("Failed to finalize archive");
    }

    const auto archive_size =
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

    std::uint64_t original_size = 0;
    auto frequencies = read_header(input, original_size);

    auto root = build_huffman_tree(frequencies);

    if (original_size > std::vector<std::uint8_t>().max_size()) {
        throw std::runtime_error("Archive is too large to decode");
    }

    std::vector<std::uint8_t> decoded;
    decoded.reserve(static_cast<std::size_t>(original_size));

    if (original_size == 0) {
        if (root) {
            throw std::runtime_error("Invalid empty archive");
        }
    } else {
        if (!root) {
            throw std::runtime_error("Missing Huffman tree");
        }

        BitReader reader(input);

        if (root->is_leaf()) {
            for (std::uint64_t i = 0; i < original_size; ++i) {
                bool bit;

                if (!reader.read_bit(bit) || bit) {
                    throw std::runtime_error(
                        "Invalid single-symbol payload"
                    );
                }

                decoded.push_back(
                    static_cast<std::uint8_t>(root->symbol)
                );
            }
        } else {
            while (decoded.size() < original_size) {
                auto node = root;

                while (!node->is_leaf()) {
                    bool bit;

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
    }

    // Verify that the decoded data matches the stored frequencies.
    auto decoded_frequencies = calculate_frequencies(decoded);

    if (decoded_frequencies != frequencies) {
        throw std::runtime_error(
            "Decoded data does not match archive frequencies"
        );
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

    return original_size;
}
