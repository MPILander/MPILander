#include <iostream>
#include <cstdlib>
#include "mpi.h"

// Simple test runner
class TestRunner {
private:
    std::string current_test;
    int tests_run = 0;
    int tests_passed = 0;

public:
    void start_test(const std::string& name) {
        current_test = name;
        tests_run++;
        std::cout << "Testing " << name << "... ";
    }

    void pass() {
        tests_passed++;
        std::cout << "PASS\n";
    }

    void fail(const std::string& msg = "") {
        std::cout << "FAIL";
        if (!msg.empty()) {
            std::cout << " - " << msg;
        }
        std::cout << "\n";
    }

    void summary() {
        std::cout << "\n========== SUMMARY ==========\n";
        std::cout << "Tests run: " << tests_run << "\n";
        std::cout << "Tests passed: " << tests_passed << "\n";
        std::cout << "Tests failed: " << (tests_run - tests_passed) << "\n";
        if (tests_passed == tests_run) {
            std::cout << "ALL TESTS PASSED!\n";
        }
    }

    bool all_passed() const {
        return tests_passed == tests_run;
    }
};

TestRunner test;

void test_mpi_finalized() {
    test.start_test("MPI_Finalized before init");
    int flag;
    int result = MPI_Finalized(&flag);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Finalized returned error");
        return;
    }
    if (flag != 0) {
        test.fail("Expected MPI not to be finalized before init");
        return;
    }
    test.pass();
}

void test_mpi_init_thread() {
    test.start_test("MPI_Init_thread");
    int argc = 0;
    char** argv = nullptr;
    int provided;
    int result = MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &provided);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Init_thread failed");
        return;
    }
    if (provided != MPI_THREAD_MULTIPLE) {
        test.fail("Expected MPI_THREAD_MULTIPLE but got different level");
        return;
    }
    test.pass();
}

void test_mpi_query_thread() {
    test.start_test("MPI_Query_thread");
    int provided;
    int result = MPI_Query_thread(&provided);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Query_thread failed");
        return;
    }
    if (provided != MPI_THREAD_MULTIPLE) {
        test.fail("Expected MPI_THREAD_MULTIPLE");
        return;
    }
    test.pass();
}

void test_mpi_initialized_after_init() {
    test.start_test("MPI_Initialized after init");
    int flag;
    int result = MPI_Initialized(&flag);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Initialized returned error");
        return;
    }
    if (flag != 1) {
        test.fail("Expected MPI to be initialized");
        return;
    }
    test.pass();
}

void test_mpi_finalized_after_init() {
    test.start_test("MPI_Finalized after init");
    int flag;
    int result = MPI_Finalized(&flag);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Finalized returned error");
        return;
    }
    if (flag != 0) {
        test.fail("Expected MPI not to be finalized after init");
        return;
    }
    test.pass();
}

void test_mpi_finalize() {
    test.start_test("MPI_Finalize");
    int result = MPI_Finalize();
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Finalize failed");
        return;
    }
    test.pass();
}

void test_mpi_finalized_after_finalize() {
    test.start_test("MPI_Finalized after finalize");
    int flag;
    int result = MPI_Finalized(&flag);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Finalized returned error");
        return;
    }
    if (flag != 1) {
        test.fail("Expected MPI to be finalized");
        return;
    }
    test.pass();
}

void test_error_conditions() {
    test.start_test("Error conditions - null pointers");
    
    // Test null pointer handling
    if (MPI_Initialized(nullptr) == MPI_SUCCESS) {
        test.fail("Expected error for null pointer to MPI_Initialized");
        return;
    }
    
    if (MPI_Finalized(nullptr) == MPI_SUCCESS) {
        test.fail("Expected error for null pointer to MPI_Finalized");
        return;
    }
    
    if (MPI_Query_thread(nullptr) == MPI_SUCCESS) {
        test.fail("Expected error for null pointer to MPI_Query_thread");
        return;
    }
    
    test.pass();
}

int main() {
    std::cout << "========== MPI Thread Support Test Suite ==========\n\n";

    // Test before initialization
    test_mpi_finalized();
    test_error_conditions();

    // Initialize MPI with threading
    test_mpi_init_thread();

    // Test after initialization
    test_mpi_initialized_after_init();
    test_mpi_finalized_after_init();
    test_mpi_query_thread();

    // Finalize MPI
    test_mpi_finalize();

    // Test after finalization
    test_mpi_finalized_after_finalize();

    test.summary();
    return test.all_passed() ? 0 : 1;
}
