/* Copyright (c) 2025 Jeff Hammond
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "mpi.h"
#include "message-queue.h"
#include <memory>
#include <unordered_map>
#include <cstdlib>
#include <mutex>
#include <atomic>

// Communicator structure
struct MPILander_Comm {
    int rank;
    int size;
    std::unique_ptr<MPILander::MessageQueue::Queue> message_queue;
    
    MPILander_Comm(int r, int s) : rank(r), size(s) {
        // Create message queue for this communicator
        message_queue = std::make_unique<MPILander::MessageQueue::Queue>();
    }
};

// Global MPI state management class
class MPI_Lander_Global_state {
private:
    mutable std::mutex state_mutex;  // Protects communicator creation/destruction only
    std::atomic<int> thread_level_provided{MPI_THREAD_MULTIPLE};  // Full thread support
    
    // Predefined communicator objects
    std::unique_ptr<MPILander_Comm> world_comm;
    std::unique_ptr<MPILander_Comm> self_comm;

public:
    // Initialize MPI and create predefined communicators
    int initialize() {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        // Create MPI_COMM_WORLD (rank 0, size 1 for single-process implementation)
        world_comm = std::make_unique<MPILander_Comm>(0, 1);
        
        // Create MPI_COMM_SELF (rank 0, size 1)
        self_comm = std::make_unique<MPILander_Comm>(0, 1);
        
        return MPI_SUCCESS;
    }
    
    // Finalize MPI and cleanup communicators
    int finalize() {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        // Clean up predefined communicators
        world_comm.reset();
        self_comm.reset();
        
        return MPI_SUCCESS;
    }
    

    
    // Lookup function for MPI_Comm objects
    MPILander_Comm* lookup_comm(MPI_Comm comm) {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        if (!comm) return nullptr;
        
        // Check for predefined communicators first
        if (comm == MPI_COMM_WORLD) {
            return world_comm.get();
        }
        if (comm == MPI_COMM_SELF) {
            return self_comm.get();
        }
        
        // For other communicators, treat the handle as a literal heap pointer
        return reinterpret_cast<MPILander_Comm*>(comm);
    }
    
    // Initialize MPI with thread support
    int initialize_thread(int required, int *provided) {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        // We always provide full thread support
        (void)required;  // Unused - we always provide MPI_THREAD_MULTIPLE
        thread_level_provided.store(MPI_THREAD_MULTIPLE);
        if (provided) {
            *provided = thread_level_provided.load();
        }
        
        // Create MPI_COMM_WORLD (rank 0, size 1 for single-process implementation)
        world_comm = std::make_unique<MPILander_Comm>(0, 1);
        
        // Create MPI_COMM_SELF (rank 0, size 1)
        self_comm = std::make_unique<MPILander_Comm>(0, 1);
        
        return MPI_SUCCESS;
    }
    
    // Query current thread support level
    int query_thread(int *provided) const {
        if (!provided) {
            return MPI_ERR_ARG;
        }
        
        *provided = thread_level_provided.load();
        return MPI_SUCCESS;
    }
};

// Custom deleter that handles magic value 0x1
struct MPI_Lander_Global_state_deleter {
    void operator()(MPI_Lander_Global_state* ptr) {
        if (ptr != reinterpret_cast<MPI_Lander_Global_state*>(0x1)) {
            delete ptr;
        }
        // If ptr == 0x1, do nothing (it's the magic finalized value)
    }
};

// Global state pointer - allocated during MPI_Init, deallocated during MPI_Finalize
// No mutex needed since MPI_Init/Finalize are called exactly once
// nullptr = not initialized, valid pointer = initialized, 0x1 = finalized
static std::unique_ptr<MPI_Lander_Global_state, MPI_Lander_Global_state_deleter> g_state{nullptr};

// Helper functions for state checking
static bool is_mpi_initialized() {
    return g_state.get() != nullptr && g_state.get() != reinterpret_cast<MPI_Lander_Global_state*>(0x1);
}

static bool is_mpi_finalized() {
    return g_state.get() == reinterpret_cast<MPI_Lander_Global_state*>(0x1);
}

// Helper function to safely access global state
static MPI_Lander_Global_state* get_global_state() {
    return g_state.get();
}

// MPI Functions Implementation

extern "C" {

int MPI_Init(int *argc, char ***argv) {
    (void)argc;  // Unused parameters
    (void)argv;
    
    // Allocate global state - MPI guarantees this is called exactly once
    if (!g_state) {
        g_state.reset(new MPI_Lander_Global_state());
    }
    
    return g_state->initialize();
}

int MPI_Finalize() {
    // MPI guarantees this is called exactly once, no mutex needed
    if (!g_state) {
        return MPI_ERR_OTHER;  // Not initialized
    }
    
    auto result = g_state->finalize();
    
    // Set magic value to indicate finalized state
    g_state.reset(reinterpret_cast<MPI_Lander_Global_state*>(0x1));
    
    return result;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank) {
    if (!is_mpi_initialized()) {
        return MPI_ERR_OTHER;
    }
    
    if (!rank) {
        return MPI_ERR_ARG;
    }
    
    auto state = get_global_state();
    auto comm_obj = state->lookup_comm(comm);
    if (!comm_obj) {
        return MPI_ERR_COMM;
    }
    
    *rank = comm_obj->rank;
    return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size) {
    if (!is_mpi_initialized()) {
        return MPI_ERR_OTHER;
    }
    
    if (!size) {
        return MPI_ERR_ARG;
    }
    
    auto state = get_global_state();
    auto comm_obj = state->lookup_comm(comm);
    if (!comm_obj) {
        return MPI_ERR_COMM;
    }
    
    *size = comm_obj->size;
    return MPI_SUCCESS;
}

int MPI_Abort(MPI_Comm comm, int errorcode) {
    (void)comm;  // Unused in this simple implementation
    
    // In a real MPI implementation, this would terminate all processes
    // For our single-process implementation, just exit
    exit(errorcode);
    return MPI_SUCCESS;  // Never reached
}

int MPI_Initialized(int *flag) {
    if (!flag) {
        return MPI_ERR_ARG;
    }
    
    *flag = is_mpi_initialized() ? 1 : 0;
    return MPI_SUCCESS;
}

int MPI_Finalized(int *flag) {
    if (!flag) {
        return MPI_ERR_ARG;
    }
    
    *flag = is_mpi_finalized() ? 1 : 0;
    return MPI_SUCCESS;
}

int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
    (void)argc;  // Unused parameters
    (void)argv;
    
    // Allocate global state - MPI guarantees this is called exactly once
    if (!g_state) {
        g_state.reset(new MPI_Lander_Global_state());
    }
    
    return g_state->initialize_thread(required, provided);
}

int MPI_Query_thread(int *provided) {
    if (!is_mpi_initialized()) {
        return MPI_ERR_OTHER;
    }
    
    auto state = get_global_state();
    return state->query_thread(provided);
}

} // extern "C"
