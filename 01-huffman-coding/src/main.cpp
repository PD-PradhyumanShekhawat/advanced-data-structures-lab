
#include "../include/huffman_codec.hpp"

#include <exception>
#include <iomanip>
#include <iostream>
#include <string>

void print_usage(const char* program) {
    std::cerr
        << "Usage:\n"
        << "  " << program
        << " compress <input-file> <output-archive>\n"
        << "  " << program
        << " decompress <input-archive> <output-file>\n";
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        print_usage(argv[0]);
        return 1;
    }

    const std::string command = argv[1];

    try {
        if (command == "compress") {
            auto stats = compress_file(argv[2], argv[3]);

            std::cout << "Compression completed.\n"
                      << "Original size: " << stats.original_size
                      << " bytes\n"
                      << "Archive size: " << stats.archive_size
                      << " bytes\n"
                      << std::fixed << std::setprecision(2)
                      << "Archive/original ratio: "
                      << stats.compression_ratio * 100.0
                      << "%\n";

            return 0;
        }

        if (command == "decompress") {
            auto size = decompress_file(argv[2], argv[3]);

            std::cout << "Decompression completed.\n"
                      << "Restored size: " << size << " bytes\n";

            return 0;
        }

        print_usage(argv[0]);
        return 1;

    } catch (const std::exception& error) {
        std::cerr << "Error: " << error.what() << '\n';
        return 2;
    }
}
