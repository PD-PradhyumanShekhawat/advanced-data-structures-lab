
#include "../include/huffman_codec.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

std::vector<std::uint8_t> read_bytes(const fs::path& path) {
    std::ifstream input(path, std::ios::binary);

    if (!input) {
        throw std::runtime_error("Cannot read benchmark file");
    }

    std::vector<std::uint8_t> data;
    char byte;

    while (input.get(byte)) {
        data.push_back(static_cast<std::uint8_t>(
            static_cast<unsigned char>(byte)
        ));
    }

    return data;
}

void write_bytes(
    const fs::path& path,
    const std::vector<std::uint8_t>& data
) {
    std::ofstream output(path, std::ios::binary);

    if (!output) {
        throw std::runtime_error("Cannot create benchmark file");
    }

    if (!data.empty()) {
        output.write(
            reinterpret_cast<const char*>(data.data()),
            static_cast<std::streamsize>(data.size())
        );
    }

    if (!output) {
        throw std::runtime_error("Failed to write benchmark file");
    }
}

std::vector<std::uint8_t> make_skewed_data(std::size_t size) {
    std::vector<std::uint8_t> data;
    data.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        if (i % 100 < 80) {
            data.push_back('A');
        } else if (i % 100 < 95) {
            data.push_back('B');
        } else {
            data.push_back(static_cast<std::uint8_t>('C' + i % 4));
        }
    }

    return data;
}

std::vector<std::uint8_t> make_uniform_data(std::size_t size) {
    std::vector<std::uint8_t> data;
    data.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        data.push_back(static_cast<std::uint8_t>(i % 256));
    }

    return data;
}

std::vector<std::uint8_t> make_text_data(std::size_t size) {
    const std::string sample =
        "Huffman coding compresses data using variable-length codes. "
        "Symbols that occur frequently receive shorter codes. "
        "This benchmark tests realistic repetitive text data. ";

    std::vector<std::uint8_t> data;
    data.reserve(size);

    while (data.size() < size) {
        for (unsigned char byte : sample) {
            if (data.size() == size) {
                break;
            }

            data.push_back(byte);
        }
    }

    return data;
}

std::vector<std::uint8_t> make_random_data(std::size_t size) {
    std::mt19937 generator(42);
    std::uniform_int_distribution<unsigned> distribution(0, 255);

    std::vector<std::uint8_t> data;
    data.reserve(size);

    for (std::size_t i = 0; i < size; ++i) {
        data.push_back(static_cast<std::uint8_t>(
            distribution(generator)
        ));
    }

    return data;
}

void benchmark_case(
    const std::string& name,
    const std::vector<std::uint8_t>& data,
    const fs::path& input,
    const fs::path& archive,
    const fs::path& restored
) {
    write_bytes(input, data);

    const auto compression_start = Clock::now();
    const auto stats = compress_file(input, archive);
    const auto compression_end = Clock::now();

    const auto decompression_start = Clock::now();
    decompress_file(archive, restored);
    const auto decompression_end = Clock::now();

    const auto restored_data = read_bytes(restored);

    if (restored_data != data) {
        throw std::runtime_error(
            "Byte-for-byte benchmark verification failed: " + name
        );
    }

    const double compression_seconds =
        std::chrono::duration<double>(
            compression_end - compression_start
        ).count();

    const double decompression_seconds =
        std::chrono::duration<double>(
            decompression_end - decompression_start
        ).count();

    const double size_mb =
        static_cast<double>(data.size()) / (1024.0 * 1024.0);

    const double compression_mbps =
        compression_seconds > 0
            ? size_mb / compression_seconds
            : 0.0;

    const double decompression_mbps =
        decompression_seconds > 0
            ? size_mb / decompression_seconds
            : 0.0;

    const double archive_ratio =
        data.empty()
            ? 0.0
            : static_cast<double>(stats.archive_size) /
              static_cast<double>(stats.original_size);

    const double savings =
        (1.0 - archive_ratio) * 100.0;

    std::cout << "\nDataset: " << name << '\n';
    std::cout << "-----------------------------\n";

    std::cout << "Original size: "
              << stats.original_size
              << " bytes\n";

    std::cout << "Archive size: "
              << stats.archive_size
              << " bytes\n";

    std::cout << "Archive/original ratio: "
              << archive_ratio * 100.0
              << "%\n";

    std::cout << "Space savings: "
              << savings
              << "%\n";

    std::cout << "Compression time: "
              << compression_seconds
              << " seconds\n";

    std::cout << "Decompression time: "
              << decompression_seconds
              << " seconds\n";

    std::cout << "Compression throughput: "
              << compression_mbps
              << " MB/s\n";

    std::cout << "Decompression throughput: "
              << decompression_mbps
              << " MB/s\n";
}

int main() {
    const fs::path input = "build/benchmark_input.bin";
    const fs::path archive = "build/benchmark.huff";
    const fs::path restored = "build/benchmark_restored.bin";

    fs::create_directories("build");

    constexpr std::size_t SIZE = 10 * 1024 * 1024;

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Huffman Archive v2 Benchmark\n";
    std::cout << "============================\n";
    std::cout << "Dataset size: 10 MB\n";

    benchmark_case(
        "Skewed distribution",
        make_skewed_data(SIZE),
        input,
        archive,
        restored
    );

    benchmark_case(
        "Uniform distribution",
        make_uniform_data(SIZE),
        input,
        archive,
        restored
    );

    benchmark_case(
        "Repeated text",
        make_text_data(SIZE),
        input,
        archive,
        restored
    );

    benchmark_case(
        "Pseudorandom bytes",
        make_random_data(SIZE),
        input,
        archive,
        restored
    );

    fs::remove(input);
    fs::remove(archive);
    fs::remove(restored);

    std::cout << "\nAll benchmark datasets verified.\n";

    return 0;
}
