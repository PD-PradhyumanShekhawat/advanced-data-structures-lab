
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g -Iinclude
RELEASE_FLAGS = -std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude
FILE_UTILS_TEST = $(BUILD_DIR)/test_file_utils

BUILD_DIR = build

MIN_HEAP_TEST = $(BUILD_DIR)/test_min_heap
HUFFMAN_TREE_TEST = $(BUILD_DIR)/test_huffman_tree
HUFFMAN_CODES_TEST = $(BUILD_DIR)/test_huffman_codes
BIT_IO_TEST = $(BUILD_DIR)/test_bit_io
HUFFMAN_CODEC_TEST = $(BUILD_DIR)/test_huffman_codec

HUFFMAN_APP = $(BUILD_DIR)/huffman
BENCHMARK = $(BUILD_DIR)/benchmark_huffman

CRC32_TEST = $(BUILD_DIR)/test_crc32
ARCHIVE_FORMAT_TEST = $(BUILD_DIR)/test_archive_format

.PHONY: all test benchmark clean

all: $(HUFFMAN_APP) test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(MIN_HEAP_TEST): tests/test_min_heap.cpp include/min_heap.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(HUFFMAN_TREE_TEST): tests/test_huffman_tree.cpp include/frequency_table.hpp include/huffman_node.hpp include/huffman_tree.hpp include/min_heap.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(HUFFMAN_CODES_TEST): tests/test_huffman_codes.cpp include/huffman_codes.hpp include/huffman_tree.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BIT_IO_TEST): tests/test_bit_io.cpp include/bit_writer.hpp include/bit_reader.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(HUFFMAN_CODEC_TEST): tests/test_huffman_codec.cpp src/huffman_codec.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(HUFFMAN_APP): src/main.cpp src/huffman_codec.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(BENCHMARK): benchmarks/benchmark_huffman.cpp src/huffman_codec.cpp | $(BUILD_DIR)
	$(CXX) $(RELEASE_FLAGS) $^ -o $@

$(CRC32_TEST): tests/test_crc32.cpp include/crc32.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(ARCHIVE_FORMAT_TEST): tests/test_archive_format.cpp include/archive_format.hpp include/frequency_table.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(FILE_UTILS_TEST): tests/test_file_utils.cpp include/file_utils.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

test: $(MIN_HEAP_TEST) $(HUFFMAN_TREE_TEST) $(HUFFMAN_CODES_TEST) $(BIT_IO_TEST) $(HUFFMAN_CODEC_TEST) $(CRC32_TEST) $(ARCHIVE_FORMAT_TEST)
	./$(MIN_HEAP_TEST)
	./$(HUFFMAN_TREE_TEST)
	./$(HUFFMAN_CODES_TEST)
	./$(BIT_IO_TEST)
	./$(HUFFMAN_CODEC_TEST)
	./$(CRC32_TEST)
	./$(ARCHIVE_FORMAT_TEST)
	./$(FILE_UTILS_TEST)

benchmark: $(BENCHMARK)
	./$(BENCHMARK)

clean:
	rm -rf $(BUILD_DIR)
