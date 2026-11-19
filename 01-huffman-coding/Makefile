
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -g

TEST_DIR = tests
BUILD_DIR = build

TEST_SOURCE = $(TEST_DIR)/test_min_heap.cpp
TEST_BINARY = $(BUILD_DIR)/test_min_heap

.PHONY: all test clean

all: test

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TEST_BINARY): $(TEST_SOURCE) include/min_heap.hpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(TEST_SOURCE) -o $(TEST_BINARY)

test: $(TEST_BINARY)
	./$(TEST_BINARY)

clean:
	rm -rf $(BUILD_DIR)
