
#include "../include/file_utils.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs = std::filesystem;

const fs::path TEST_DIR =
    fs::temp_directory_path() / "huffman_file_utils_tests";

void prepare() {
    fs::create_directories(TEST_DIR);
}

void cleanup() {
    if (fs::exists(TEST_DIR)) {
        fs::remove_all(TEST_DIR);
    }
}

void create_file(const fs::path& path) {
    std::ofstream output(path);

    if (!output) {
        throw std::runtime_error("Cannot create test file");
    }

    output << "test";
}

void test_valid_operation() {
    const fs::path input = TEST_DIR / "input.txt";
    const fs::path output = TEST_DIR / "output.txt";

    create_file(input);

    FileUtils::validate_file_operation(input, output);
}

void test_missing_input() {
    const fs::path input = TEST_DIR / "missing.txt";
    const fs::path output = TEST_DIR / "output.txt";

    bool rejected = false;

    try {
        FileUtils::validate_file_operation(input, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_same_path_rejected() {
    const fs::path input = TEST_DIR / "same.txt";

    create_file(input);

    bool rejected = false;

    try {
        FileUtils::validate_file_operation(input, input);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_directory_as_input_rejected() {
    const fs::path directory = TEST_DIR / "directory";
    const fs::path output = TEST_DIR / "output.txt";

    fs::create_directories(directory);

    bool rejected = false;

    try {
        FileUtils::validate_file_operation(directory, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_directory_as_output_rejected() {
    const fs::path input = TEST_DIR / "input2.txt";
    const fs::path output = TEST_DIR / "existing_directory";

    create_file(input);
    fs::create_directories(output);

    bool rejected = false;

    try {
        FileUtils::validate_file_operation(input, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_missing_output_parent_rejected() {
    const fs::path input = TEST_DIR / "input3.txt";
    const fs::path output =
        TEST_DIR / "nonexistent_parent" / "output.txt";

    create_file(input);

    bool rejected = false;

    try {
        FileUtils::validate_file_operation(input, output);
    } catch (const std::exception&) {
        rejected = true;
    }

    assert(rejected);
}

void test_normalization() {
    const fs::path input = TEST_DIR / "input.txt";

    const auto normalized =
        FileUtils::normalized_path(input / ".." / "input.txt");

    assert(normalized == FileUtils::normalized_path(input));
}

int main() {
    prepare();

    test_valid_operation();
    test_missing_input();
    test_same_path_rejected();
    test_directory_as_input_rejected();
    test_directory_as_output_rejected();
    test_missing_output_parent_rejected();
    test_normalization();

    cleanup();

    std::cout << "All file utility tests passed.\n";

    return 0;
}
