
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g

TEST_DIR = tests
BUILD_DIR = build

MIN_HEAP_TEST = $(TEST_DIR)/test_min_heap.cpp
HUFFMAN_TEST = $(TEST_DIR)/test_huffman_tree.cpp

MIN_HEAP_BINARY = $(BUILD_DIR)/test_min_heap
HUFFMAN_BINARY = $(BUILD_DIR)/test_huffman_tree

.PHONY: all test clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(MIN_HEAP_BINARY): $(MIN_HEAP_TEST) include/min_heap.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(MIN_HEAP_TEST) -o $(MIN_HEAP_BINARY)

$(HUFFMAN_BINARY): $(HUFFMAN_TEST) include/frequency_table.hpp include/huffman_node.hpp include/huffman_tree.hpp include/min_heap.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(HUFFMAN_TEST) -o $(HUFFMAN_BINARY)

test: $(MIN_HEAP_BINARY) $(HUFFMAN_BINARY)
	./$(MIN_HEAP_BINARY)
	./$(HUFFMAN_BINARY)

clean:
	rm -rf $(BUILD_DIR)
