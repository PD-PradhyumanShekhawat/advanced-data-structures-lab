
#pragma once

#include <cstdint>
#include <filesystem>

struct CompressionStats {
    std::uint64_t original_size;
    std::uint64_t archive_size;
    double compression_ratio;
};

CompressionStats compress_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path
);

std::uint64_t decompress_file(
    const std::filesystem::path& input_path,
    const std::filesystem::path& output_path
);
