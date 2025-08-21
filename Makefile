# MPILander Makefile
# Based on CMakeLists.txt configuration

# Project information
PROJECT_NAME = MPILander
VERSION = 0.1.0
MPI_STANDARD_VERSION = 3.1

# Compiler and flags
CXX = g++
CXXFLAGS = -std=c++17 -fPIC -Wall -Wextra
INCLUDES = -Iinclude

# Build type (Release or Debug)
BUILD_TYPE ?= Release

ifeq ($(BUILD_TYPE),Debug)
    CXXFLAGS += -g -O0 -DDEBUG
else
    CXXFLAGS += -O3 -DNDEBUG
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin
OBJ_DIR = $(BUILD_DIR)/obj

# Installation directories
PREFIX ?= /usr/local
INSTALL_LIBDIR = $(PREFIX)/lib
INSTALL_INCLUDEDIR = $(PREFIX)/include
INSTALL_BINDIR = $(PREFIX)/bin

# Source files
CPP_SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
CC_SOURCES = $(wildcard $(SRC_DIR)/*.cc)
SOURCES = $(CPP_SOURCES) $(CC_SOURCES)
HEADERS = $(wildcard $(INCLUDE_DIR)/*.h)
OBJECTS = $(CPP_SOURCES:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o) $(CC_SOURCES:$(SRC_DIR)/%.cc=$(OBJ_DIR)/%.o)

# Library names
STATIC_LIB = $(LIB_DIR)/libmpi.a
SHARED_LIB = $(LIB_DIR)/libmpi.so
LIBRARY_NAME = MPILander

# Default target
.PHONY: all
all: directories $(STATIC_LIB) $(SHARED_LIB)

# Create necessary directories
.PHONY: directories
directories:
	@mkdir -p $(BUILD_DIR) $(LIB_DIR) $(BIN_DIR) $(OBJ_DIR)

# Compile C++ object files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Compile C++ object files (*.cc)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Build static library
$(STATIC_LIB): $(OBJECTS)
	ar rcs $@ $^
	@echo "Built static library: $@"

# Build shared library
$(SHARED_LIB): $(OBJECTS)
	$(CXX) -shared -o $@ $^
	@echo "Built shared library: $@"

# Build development tests
.PHONY: test
test: $(STATIC_LIB)
	$(MAKE) -C development

# Build tests in test directory
.PHONY: tests
tests: $(STATIC_LIB)
	$(MAKE) -C test

# Clean build artifacts
.PHONY: clean
clean:
	-rm -rf $(BUILD_DIR)
	-$(MAKE) -C development clean 2>/dev/null || true
	-$(MAKE) -C test clean 2>/dev/null || true
	@echo "Cleaned build artifacts"

# Install libraries and headers
.PHONY: install
install: all
	@mkdir -p $(INSTALL_LIBDIR) $(INSTALL_INCLUDEDIR)
	cp $(STATIC_LIB) $(INSTALL_LIBDIR)/
	cp $(SHARED_LIB) $(INSTALL_LIBDIR)/
	cp $(INCLUDE_DIR)/*.h $(INSTALL_INCLUDEDIR)/
	@echo "Installed to $(PREFIX)"

# Uninstall
.PHONY: uninstall
uninstall:
	-rm -f $(INSTALL_LIBDIR)/libmpi.a
	-rm -f $(INSTALL_LIBDIR)/libmpi.so
	-rm -f $(INSTALL_INCLUDEDIR)/mpi.h
	@echo "Uninstalled from $(PREFIX)"

# Debug build
.PHONY: debug
debug:
	$(MAKE) BUILD_TYPE=Debug

# Release build
.PHONY: release
release:
	$(MAKE) BUILD_TYPE=Release

# Show help
.PHONY: help
help:
	@echo "MPILander Makefile"
	@echo "Available targets:"
	@echo "  all       - Build both static and shared libraries (default)"
	@echo "  debug     - Build with debug flags"
	@echo "  release   - Build with release flags (default)"
	@echo "  test      - Build development tests"
	@echo "  tests     - Build unit tests"
	@echo "  clean     - Remove build artifacts"
	@echo "  install   - Install libraries and headers"
	@echo "  uninstall - Remove installed files"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE - Debug or Release (default: Release)"
	@echo "  PREFIX     - Installation prefix (default: /usr/local)"
	@echo "  CXX        - C++ compiler (default: g++)"

# Print project information
.PHONY: info
info:
	@echo "Project: $(PROJECT_NAME) v$(VERSION)"
	@echo "MPI Standard: $(MPI_STANDARD_VERSION)"
	@echo "Build Type: $(BUILD_TYPE)"
	@echo "Compiler: $(CXX)"
	@echo "Flags: $(CXXFLAGS)"
	@echo "Sources: $(SOURCES)"
	@echo "Headers: $(HEADERS)"
	@echo "Objects: $(OBJECTS)"

# List source files
.PHONY: list-sources
list-sources:
	@echo "C++ Sources (.cpp):"
	@for file in $(CPP_SOURCES); do echo "  $$file"; done
	@echo "C++ Sources (.cc):"
	@for file in $(CC_SOURCES); do echo "  $$file"; done
	@echo "Headers:"
	@for file in $(HEADERS); do echo "  $$file"; done
