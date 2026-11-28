
#pragma once

#include <filesystem>
#include <stdexcept>

namespace FileUtils {

inline std::filesystem::path normalized_path(
    const std::filesystem::path& path
) {
    if (path.empty()) {
        throw std::invalid_argument("File path cannot be empty");
    }

    return std::filesystem::absolute(path).lexically_normal();
}

inline void validate_input_file(
    const std::filesystem::path& input
) {
    if (!std::filesystem::exists(input)) {
        throw std::runtime_error("Input file does not exist");
    }

    if (!std::filesystem::is_regular_file(input)) {
        throw std::runtime_error("Input is not a regular file");
    }
}

inline void validate_output_file(
    const std::filesystem::path& output
) {
    const auto normalized = normalized_path(output);

    if (std::filesystem::exists(normalized) &&
        std::filesystem::is_directory(normalized)) {
        throw std::runtime_error("Output path is a directory");
    }

    const auto parent = normalized.parent_path();

    if (!parent.empty() && !std::filesystem::exists(parent)) {
        throw std::runtime_error("Output directory does not exist");
    }
}

inline void validate_distinct_paths(
    const std::filesystem::path& input,
    const std::filesystem::path& output
) {
    if (normalized_path(input) == normalized_path(output)) {
        throw std::runtime_error(
            "Input and output paths must be different"
        );
    }
}

inline void validate_file_operation(
    const std::filesystem::path& input,
    const std::filesystem::path& output
) {
    validate_input_file(input);
    validate_output_file(output);
    validate_distinct_paths(input, output);
}

} // namespace FileUtils
