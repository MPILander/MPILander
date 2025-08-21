/* Copyright (c) 2025 Jeff Hammond
 *
 * MIT License
 */

#include <iostream>
#include <cassert>
#include <cstring>
#include <vector>
#include <string>

#include "mpi.h"

// Test utilities
class TestRunner {
private:
    int total_tests = 0;
    int passed_tests = 0;
    std::string current_test;

public:
    void start_test(const std::string& name) {
        current_test = name;
        total_tests++;
        std::cout << "Running: " << name << "... ";
    }
    
    void pass() {
        passed_tests++;
        std::cout << "PASS\n";
    }
    
    void fail(const std::string& msg = "") {
        std::cout << "FAIL";
        if (!msg.empty()) {
            std::cout << " - " << msg;
        }
        std::cout << "\n";
        
        // Call MPI_Abort on test failure
        int flag;
        if (MPI_Initialized(&flag) == MPI_SUCCESS && flag) {
            std::cerr << "Test failed: " << current_test;
            if (!msg.empty()) {
                std::cerr << " - " << msg;
            }
            std::cerr << "\nAborting MPI execution...\n";
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }
    
    void assert_equal(int expected, int actual, const std::string& msg = "") {
        if (expected != actual) {
            fail("Expected " + std::to_string(expected) + ", got " + std::to_string(actual) + 
                 (msg.empty() ? "" : " (" + msg + ")"));
        } else {
            pass();
        }
    }
    
    void assert_true(bool condition, const std::string& msg = "") {
        if (!condition) {
            fail(msg.empty() ? "Condition failed" : msg);
        } else {
            pass();
        }
    }
    
    void summary() {
        std::cout << "\n=== Test Summary ===\n";
        std::cout << "Total tests: " << total_tests << "\n";
        std::cout << "Passed: " << passed_tests << "\n";
        std::cout << "Failed: " << (total_tests - passed_tests) << "\n";
        std::cout << "Success rate: " << (100.0 * passed_tests / total_tests) << "%\n";
    }
    
    bool all_passed() const {
        return passed_tests == total_tests;
    }
};

// Global test runner
TestRunner test;

// Global MPI state
static bool mpi_initialized = false;

// Initialize MPI once for all tests
bool init_mpi() {
    if (!mpi_initialized) {
        int argc = 0;
        char** argv = nullptr;
        int result = MPI_Init(&argc, &argv);
        mpi_initialized = (result == MPI_SUCCESS);
        return mpi_initialized;
    }
    return true;
}

// Finalize MPI once after all tests
bool finalize_mpi() {
    if (mpi_initialized) {
        int result = MPI_Finalize();
        mpi_initialized = false;
        return (result == MPI_SUCCESS);
    }
    return true;
}

// Test basic MPI queries
void test_mpi_queries() {
    // Test MPI_Comm_rank
    test.start_test("MPI_Comm_rank");
    int rank;
    int result = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    test.assert_equal(MPI_SUCCESS, result);
    
    // Test MPI_Comm_size  
    test.start_test("MPI_Comm_size");
    int size;
    result = MPI_Comm_size(MPI_COMM_WORLD, &size);
    test.assert_equal(MPI_SUCCESS, result);
    
    // For single-process implementation, rank should be 0 and size should be 1
    test.start_test("Single process rank");
    test.assert_equal(0, rank);
    
    test.start_test("Single process size");
    test.assert_equal(1, size);
}

// Test basic send/recv with exact matching
void test_basic_send_recv() {
    test.start_test("Basic MPI_Send/MPI_Recv");
    
    const int tag = 42;
    const int count = 10;
    int send_buffer[count];
    int recv_buffer[count];
    
    // Initialize send buffer
    for (int i = 0; i < count; i++) {
        send_buffer[i] = i * i;  // 0, 1, 4, 9, 16, ...
        recv_buffer[i] = -1;     // Initialize to invalid values
    }
    
    // Send message (to self in single-process implementation)
    int result = MPI_Send(send_buffer, count, MPI_INT, 0, tag, MPI_COMM_WORLD);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Send failed with error code " + std::to_string(result));
        return;
    }
    
    // Receive message
    MPI_Status status;
    result = MPI_Recv(recv_buffer, count, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
    if (result != MPI_SUCCESS) {
        test.fail("MPI_Recv failed with error code " + std::to_string(result));
        return;
    }
    
    // Verify data integrity
    bool data_correct = true;
    for (int i = 0; i < count; i++) {
        if (send_buffer[i] != recv_buffer[i]) {
            data_correct = false;
            break;
        }
    }
    
    if (data_correct) {
        test.pass();
    } else {
        test.fail("Data corruption detected");
    }
}

// Test wildcard receiving with MPI_ANY_SOURCE
void test_any_source() {

    
    test.start_test("MPI_ANY_SOURCE wildcard");
    
    const int tag = 100;
    int data = 12345;
    int recv_data = 0;
    
    // Send message
    MPI_Send(&data, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    
    // Receive with wildcard source
    MPI_Status status;
    int result = MPI_Recv(&recv_data, 1, MPI_INT, MPI_ANY_SOURCE, tag, MPI_COMM_WORLD, &status);
    
    if (result == MPI_SUCCESS && recv_data == data && status.MPI_SOURCE == 0) {
        test.pass();
    } else {
        test.fail("Wildcard source matching failed");
    }
    
}

// Test wildcard receiving with MPI_ANY_TAG
void test_any_tag() {

    
    test.start_test("MPI_ANY_TAG wildcard");
    
    const int tag = 999;
    int data = 54321;
    int recv_data = 0;
    
    // Send message
    MPI_Send(&data, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    
    // Receive with wildcard tag
    MPI_Status status;
    int result = MPI_Recv(&recv_data, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    if (result == MPI_SUCCESS && recv_data == data && status.MPI_TAG == tag) {
        test.pass();
    } else {
        test.fail("Wildcard tag matching failed");
    }
    
}

// Test FIFO message ordering
void test_fifo_ordering() {

    
    test.start_test("FIFO message ordering");
    
    const int num_messages = 5;
    const int tag = 200;
    
    // Send multiple messages with same envelope
    for (int i = 0; i < num_messages; i++) {
        MPI_Send(&i, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    }
    
    // Receive messages - should come in FIFO order
    bool fifo_correct = true;
    for (int expected = 0; expected < num_messages; expected++) {
        int received;
        MPI_Status status;
        MPI_Recv(&received, 1, MPI_INT, 0, tag, MPI_COMM_WORLD, &status);
        
        if (received != expected) {
            fifo_correct = false;
            break;
        }
    }
    
    if (fifo_correct) {
        test.pass();
    } else {
        test.fail("Messages not received in FIFO order");
    }
    
}

// Test message status information
void test_message_status() {

    
    test.start_test("MPI_Status information");
    
    const int tag = 300;
    const int source = 0;
    int data = 777;
    int recv_data = 0;
    
    // Send message
    MPI_Send(&data, 1, MPI_INT, source, tag, MPI_COMM_WORLD);
    
    // Receive and check status
    MPI_Status status;
    MPI_Recv(&recv_data, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
    
    bool status_correct = (status.MPI_SOURCE == source && 
                          status.MPI_TAG == tag && 
                          status.MPI_ERROR == MPI_SUCCESS);
    
    if (status_correct && recv_data == data) {
        test.pass();
    } else {
        test.fail("Status information incorrect");
    }
    
}

// Test different data types
void test_different_datatypes() {

    
    // Test char data
    test.start_test("MPI_CHAR datatype");
    char send_char = 'A';
    char recv_char = 0;
    MPI_Send(&send_char, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD);
    MPI_Status status;
    MPI_Recv(&recv_char, 1, MPI_CHAR, 0, 1, MPI_COMM_WORLD, &status);
    test.assert_equal(send_char, recv_char);
    
    // Test float data
    test.start_test("MPI_FLOAT datatype");
    float send_float = 3.14159f;
    float recv_float = 0.0f;
    MPI_Send(&send_float, 1, MPI_FLOAT, 0, 2, MPI_COMM_WORLD);
    MPI_Recv(&recv_float, 1, MPI_FLOAT, 0, 2, MPI_COMM_WORLD, &status);
    test.assert_true(send_float == recv_float, "Float values match");
    
    // Test double data
    test.start_test("MPI_DOUBLE datatype");
    double send_double = 2.718281828;
    double recv_double = 0.0;
    MPI_Send(&send_double, 1, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
    MPI_Recv(&recv_double, 1, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD, &status);
    test.assert_true(send_double == recv_double, "Double values match");
    
}

// Test large message transfers
void test_large_messages() {

    
    test.start_test("Large message transfer");
    
    const int large_count = 1000;
    std::vector<int> send_buffer(large_count);
    std::vector<int> recv_buffer(large_count, -1);
    
    // Initialize with pattern
    for (int i = 0; i < large_count; i++) {
        send_buffer[i] = i * 2 + 1;  // Odd numbers
    }
    
    // Send large message
    MPI_Send(send_buffer.data(), large_count, MPI_INT, 0, 400, MPI_COMM_WORLD);
    
    // Receive large message
    MPI_Status status;
    MPI_Recv(recv_buffer.data(), large_count, MPI_INT, 0, 400, MPI_COMM_WORLD, &status);
    
    // Verify all data
    bool all_correct = true;
    for (int i = 0; i < large_count; i++) {
        if (send_buffer[i] != recv_buffer[i]) {
            all_correct = false;
            break;
        }
    }
    
    if (all_correct) {
        test.pass();
    } else {
        test.fail("Large message data corruption");
    }
    
}

// Test error conditions
void test_error_conditions() {

    
    test.start_test("Invalid communicator");
    int data = 123;
    MPI_Comm invalid_comm = (MPI_Comm)(-999);  // Invalid communicator
    int result = MPI_Send(&data, 1, MPI_INT, 0, 1, invalid_comm);
    test.assert_true(result != MPI_SUCCESS, "Should reject invalid communicator");
    
    test.start_test("Invalid rank");
    result = MPI_Send(&data, 1, MPI_INT, 999, 1, MPI_COMM_WORLD);  // Invalid rank
    test.assert_true(result != MPI_SUCCESS, "Should reject invalid rank");
    
}

int main(int /* argc */, char* /* argv */[]) {
    std::cout << "=== MPI Send/Recv Comprehensive Test Suite ===\n\n";
    
    // Initialize MPI once for all tests
    test.start_test("MPI Initialization");
    if (!init_mpi()) {
        std::cerr << "CRITICAL ERROR: Failed to initialize MPI\n";
        std::cerr << "Cannot run MPI tests without proper initialization\n";
        return 1;
    }
    test.pass();
    
    // Run all tests
    test_mpi_queries();
    test_basic_send_recv();
    test_any_source();
    test_any_tag();
    test_fifo_ordering();
    test_message_status();
    test_different_datatypes();
    test_large_messages();
    test_error_conditions();
    
    // Finalize MPI once after all tests
    test.start_test("MPI Finalization");
    if (!finalize_mpi()) {
        std::cerr << "FAIL - Failed to finalize MPI\n";
        return 1;
    }
    test.pass();
    
    // Show results
    test.summary();
    
    return test.all_passed() ? 0 : 1;
}
