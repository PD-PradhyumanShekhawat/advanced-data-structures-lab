
#include "../include/huffman_codec.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace fs = std::filesystem;
using Clock = std::chrono::steady_clock;

int main() {
    const fs::path input = "build/benchmark_input.bin";
    const fs::path archive = "build/benchmark.huff";
    const fs::path restored = "build/benchmark_restored.bin";

    fs::create_directories("build");

    // Generate a reproducible 10 MB input with a skewed distribution.
    {
        std::ofstream output(input, std::ios::binary);

        if (!output) {
            throw std::runtime_error("Cannot create benchmark input");
        }

        constexpr std::size_t SIZE = 10 * 1024 * 1024;

        for (std::size_t i = 0; i < SIZE; ++i) {
            unsigned char byte;

            if (i % 100 < 70) {
                byte = 'A';
            } else if (i % 100 < 90) {
                byte = 'B';
            } else {
                byte = static_cast<unsigned char>('C' + (i % 3));
            }

            output.put(static_cast<char>(byte));
        }
    }

    auto start_compress = Clock::now();

    auto stats = compress_file(input, archive);

    auto end_compress = Clock::now();

    auto start_decompress = Clock::now();

    decompress_file(archive, restored);

    auto end_decompress = Clock::now();

    const double compression_seconds =
        std::chrono::duration<double>(
            end_compress - start_compress
        ).count();

    const double decompression_seconds =
        std::chrono::duration<double>(
            end_decompress - start_decompress
        ).count();

    const double compression_mbps =
        compression_seconds > 0
            ? stats.original_size / compression_seconds / (1024.0 * 1024.0)
            : 0.0;

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "\nHuffman Compression Benchmark\n";
    std::cout << "-----------------------------\n";
    std::cout << "Original size: "
              << stats.original_size / (1024.0 * 1024.0)
              << " MB\n";

    std::cout << "Archive size: "
              << stats.archive_size / (1024.0 * 1024.0)
              << " MB\n";

    std::cout << "Archive/original ratio: "
              << stats.compression_ratio * 100.0
              << "%\n";

    std::cout << "Compression time: "
              << compression_seconds << " seconds\n";

    std::cout << "Decompression time: "
              << decompression_seconds << " seconds\n";

    std::cout << "Compression throughput: "
              << compression_mbps << " MB/s\n";

    if (fs::file_size(input) != fs::file_size(restored)) {
        throw std::runtime_error("Benchmark verification failed");
    }

    std::cout << "Benchmark completed.\n";

    fs::remove(input);
    fs::remove(archive);
    fs::remove(restored);

    return 0;
}
