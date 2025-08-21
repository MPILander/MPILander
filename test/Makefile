# MPI Lander Test Makefile

CXX = g++
CXXFLAGS = -std=c++17 -g -Wall -Wextra -Wno-unused
INCLUDES = -I../include
LIBDIR = ../build/lib
LIBS = -L$(LIBDIR) -lmpi

# Test programs
TESTS = send_recv_test
TEST_SOURCES = $(addsuffix .cpp, $(TESTS))
TEST_BINARIES = $(TESTS)

.PHONY: all clean run test check

all: $(TEST_BINARIES)

# Build test programs
%: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LIBS) -o $@

# Run all tests
run: all
	@echo "Running MPI Send/Recv tests..."
	@for test in $(TEST_BINARIES); do \
		echo ""; \
		echo "=== Running $$test ==="; \
		./$$test; \
		if [ $$? -eq 0 ]; then \
			echo "$$test: PASSED"; \
		else \
			echo "$$test: FAILED"; \
		fi; \
	done

# Alias for run
test: run
check: run

# Clean test binaries
clean:
	rm -f $(TEST_BINARIES)
	rm -f *.o
	rm -rf *.dSYM

# Show available targets
help:
	@echo "Available targets:"
	@echo "  all     - Build all test programs"
	@echo "  run     - Build and run all tests"
	@echo "  test    - Alias for run"
	@echo "  check   - Alias for run"
	@echo "  clean   - Remove test binaries"
	@echo "  help    - Show this help"

# List test programs
list:
	@echo "Test programs:"
	@for test in $(TEST_BINARIES); do echo "  $$test"; done
