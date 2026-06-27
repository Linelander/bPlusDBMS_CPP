CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

HEADERS := $(wildcard *.h)

TESTS_DIR := Tests
# Only compile test files that contain a main() function
TEST_SRCS := Tests/TestBytes.cpp Tests/TestMain.cpp
TEST_BINS := $(TEST_SRCS:$(TESTS_DIR)/%.cpp=build/%)

BUILD_DIR := build

.PHONY: all clean test

all: $(TEST_BINS)

build/%: $(TESTS_DIR)/%.cpp $(HEADERS) | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -I. $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

test: $(TEST_BINS)
	@echo "=== Running Tests ==="
	@for bin in $(TEST_BINS); do \
		echo "--- $$bin ---"; \
		$$bin && echo "PASS" || echo "FAIL"; \
	done

clean:
	rm -rf $(BUILD_DIR)
	rm -rf table/
