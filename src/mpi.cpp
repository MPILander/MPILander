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
#include <complex>

enum MPILander_Combiner {
    NAMED = 101,           // MPI_COMBINER_NAMED
    DUP = 102,             // MPI_COMBINER_DUP
    CONTIGUOUS = 103,      // MPI_COMBINER_CONTIGUOUS
    VECTOR = 104,          // MPI_COMBINER_VECTOR
    HVECTOR = 105,         // MPI_COMBINER_HVECTOR
    INDEXED = 106,         // MPI_COMBINER_INDEXED
    HINDEXED = 107,        // MPI_COMBINER_HINDEXED
    INDEXED_BLOCK = 108,   // MPI_COMBINER_INDEXED_BLOCK
    HINDEXED_BLOCK = 109,  // MPI_COMBINER_HINDEXED_BLOCK
    STRUCT = 110,          // MPI_COMBINER_STRUCT
    SUBARRAY = 111,        // MPI_COMBINER_SUBARRAY
    DARRAY = 112,          // MPI_COMBINER_DARRAY
    F90_INTEGER = 113,     // MPI_COMBINER_F90_INTEGER
    F90_REAL = 114,        // MPI_COMBINER_F90_REAL
    F90_COMPLEX = 115,     // MPI_COMBINER_F90_COMPLEX
    RESIZED = 116,         // MPI_COMBINER_RESIZED
    VALUE_INDEX = 117      // MPI_COMBINER_VALUE_INDEX
};

static int MPILander_Type_is_builtin(MPI_Datatype datatype) {
    const auto ip = reinterpret_cast<intptr_t>(datatype);
    const auto i = static_cast<int>(ip);
    return (513 <= i && i <= 1023);
}

static int MPILander_Type_size(MPI_Datatype datatype) {
    const auto ip = reinterpret_cast<intptr_t>(datatype);
    const auto i = static_cast<int>(ip);
    switch (i) {
        case 0x00000247: /* MPI_BYTE */ return 1;
        case 0x00000243: /* MPI_CHAR */ return sizeof(char);
        case 0x00000244: /* MPI_SIGNED_CHAR */ return sizeof(signed char);
        case 0x00000245: /* MPI_UNSIGNED_CHAR */ return sizeof(unsigned char);
        case 0x00000218: /* MPI_LOGICAL */ return sizeof(int);
        case 0x0000021a: /* MPI_REAL */ return sizeof(float);
        case 0x0000021c: /* MPI_DOUBLE_PRECISION */ return sizeof(double);
        case 0x00000217: /* MPI_CXX_DOUBLE_COMPLEX */ return sizeof(std::complex<double>);
        case 0x00000208: /* MPI_SHORT */ return sizeof(short);
        case 0x00000209: /* MPI_INT */ return sizeof(int);
        case 0x0000020a: /* MPI_LONG */ return sizeof(long);
        case 0x00000210: /* MPI_FLOAT */ return sizeof(float);
        case 0x00000214: /* MPI_DOUBLE */ return sizeof(double);
        case 0x00000220: /* MPI_LONG_DOUBLE */ return sizeof(long double);
        case 0x0000020b: /* MPI_LONG_LONG */ return sizeof(long long);
        default: return -1;
    }
}

// MPI Datatype management class
class MPILander_Type {

private:
    bool is_builtin_;
    size_t size_;
    MPILander_Combiner combiner_;

public:
    
};

// Communicator structure
struct MPILander_Comm {
    int rank;
    int size;
    std::unique_ptr<MPILander::MessageQueue::Queue> message_queue;
    
    MPILander_Comm(int r, int s) : rank(r), size(s) {
        // Create message queue for this communicator
        message_queue = std::make_unique<MPILander::MessageQueue::Queue>();
    }
    
    // Copy constructor for MPI_Comm_dup
    MPILander_Comm(const MPILander_Comm& other) : rank(other.rank), size(other.size) {
        // Create a new message queue for the duplicated communicator
        // Note: We don't copy the messages, just create a fresh queue
        message_queue = std::make_unique<MPILander::MessageQueue::Queue>();
    }
    
    // Disable assignment operator (not needed for MPI semantics)
    MPILander_Comm& operator=(const MPILander_Comm&) = delete;
};

// Forward declaration
static int return_or_abort(int errorcode);

// Global MPI state management class
class MPI_Lander_Global_state {
private:
    mutable std::mutex state_mutex;  // Protects communicator creation/destruction only
    
    // Predefined communicator objects
    std::unique_ptr<MPILander_Comm> world_comm;
    std::unique_ptr<MPILander_Comm> self_comm;

public:
    bool errors_return;

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
        if (provided) {
            *provided = MPI_THREAD_MULTIPLE;
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
            return return_or_abort(MPI_ERR_ARG);
        }
        
        *provided = MPI_THREAD_MULTIPLE;
        return MPI_SUCCESS;
    }
    
    // Duplicate a communicator
    MPI_Comm duplicate_comm(MPI_Comm comm) {
        std::lock_guard<std::mutex> lock(state_mutex);
        
        auto* comm_obj = lookup_comm(comm);
        if (!comm_obj) {
            return MPI_COMM_NULL;
        }
        
        // Create a new communicator using the copy constructor
        auto* new_comm = new MPILander_Comm(*comm_obj);
        
        // Return the new communicator as a handle (raw pointer cast to MPI_Comm)
        return reinterpret_cast<MPI_Comm>(new_comm);
    }
    
    // Free a user-created communicator
    int free_comm(MPI_Comm* comm) {
        if (!comm || *comm == MPI_COMM_NULL) {
            return return_or_abort(MPI_ERR_ARG);
        }
        
        // Cannot free predefined communicators
        if (*comm == MPI_COMM_WORLD || *comm == MPI_COMM_SELF) {
            return return_or_abort(MPI_ERR_COMM);
        }
        
        std::lock_guard<std::mutex> lock(state_mutex);
        
        // Delete the user-created communicator
        auto* comm_obj = reinterpret_cast<MPILander_Comm*>(*comm);
        delete comm_obj;
        *comm = MPI_COMM_NULL;
        
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

// Implementation of return_or_abort
static int return_or_abort(int errorcode) {
    if (get_global_state()->errors_return) {
        return errorcode;
    } else {
        std::exit(errorcode);
    }
}

// MPI Functions Implementation

extern "C" {

int PMPI_Init(int *argc, char ***argv) {
    (void)argc;  // Unused parameters
    (void)argv;
    
    // Allocate global state - MPI guarantees this is called exactly once
    if (!g_state) {
        g_state.reset(new MPI_Lander_Global_state());
    }
    
    return return_or_abort(g_state->initialize());
}

int PMPI_Finalize() {
    // MPI guarantees this is called exactly once, no mutex needed
    if (!g_state) {
        return return_or_abort(MPI_ERR_OTHER);  // Not initialized
    }
    
    auto result = g_state->finalize();
    
    // Set magic value to indicate finalized state
    g_state.reset(reinterpret_cast<MPI_Lander_Global_state*>(0x1));
    
    return return_or_abort(result);
}

int PMPI_Comm_rank(MPI_Comm comm, int *rank) {
    if (!is_mpi_initialized()) {
        return return_or_abort(MPI_ERR_OTHER);
    }
    
    if (!rank) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    auto state = get_global_state();
    auto comm_obj = state->lookup_comm(comm);
    if (!comm_obj) {
        return return_or_abort(MPI_ERR_COMM);
    }
    
    *rank = comm_obj->rank;
    return MPI_SUCCESS;
}

int PMPI_Comm_size(MPI_Comm comm, int *size) {
    if (!is_mpi_initialized()) {
        return return_or_abort(MPI_ERR_OTHER);
    }
    
    if (!size) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    auto state = get_global_state();
    auto comm_obj = state->lookup_comm(comm);
    if (!comm_obj) {
        return return_or_abort(MPI_ERR_COMM);
    }
    
    *size = comm_obj->size;
    return MPI_SUCCESS;
}

int PMPI_Abort(MPI_Comm comm, int errorcode) {
    (void)comm;  // Unused in this simple implementation
    
    // In a real MPI implementation, this would terminate all processes
    // For our single-process implementation, just exit
    exit(errorcode);
    return MPI_SUCCESS;  // Never reached
}

int PMPI_Initialized(int *flag) {
    if (!flag) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    *flag = is_mpi_initialized() ? 1 : 0;
    return MPI_SUCCESS;
}

int PMPI_Finalized(int *flag) {
    if (!flag) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    *flag = is_mpi_finalized() ? 1 : 0;
    return MPI_SUCCESS;
}

int PMPI_Init_thread(int *argc, char ***argv, int required, int *provided) {
    (void)argc;  // Unused parameters
    (void)argv;
    
    // Allocate global state - MPI guarantees this is called exactly once
    if (!g_state) {
        g_state.reset(new MPI_Lander_Global_state());
    }
    
    return g_state->initialize_thread(required, provided);
}

int PMPI_Query_thread(int *provided) {
    if (!is_mpi_initialized()) {
        return return_or_abort(MPI_ERR_OTHER);
    }
    
    *provided = MPI_THREAD_MULTIPLE;
    return MPI_SUCCESS;
}

int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) {
    if (!is_mpi_initialized()) {
        return MPI_ERR_OTHER;
    }
    
    if (!newcomm) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    auto state = get_global_state();
    *newcomm = state->duplicate_comm(comm);
    
    if (*newcomm == MPI_COMM_NULL) {
        return return_or_abort(MPI_ERR_COMM);
    }
    
    return MPI_SUCCESS;
}

int PMPI_Comm_free(MPI_Comm *comm) {
    if (!is_mpi_initialized()) {
        return return_or_abort(MPI_ERR_OTHER);
    }
    
    auto state = get_global_state();
    return state->free_comm(comm);
}

int PMPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result) {
    if (!is_mpi_initialized()) {
        return return_or_abort(MPI_ERR_OTHER);
    }
    
    if (!result) {
        return return_or_abort(MPI_ERR_ARG);
    }
    
    // Simple comparison - same handle means identical
    if (comm1 == comm2) {
        *result = MPI_IDENT;
    } else {
        auto state = get_global_state();
        auto* comm1_obj = state->lookup_comm(comm1);
        auto* comm2_obj = state->lookup_comm(comm2);
        
        if (!comm1_obj || !comm2_obj) {
            return return_or_abort(MPI_ERR_COMM);
        }
        
        // Check if they have the same rank and size (similar communicators)
        if (comm1_obj->rank == comm2_obj->rank && comm1_obj->size == comm2_obj->size) {
            *result = MPI_SIMILAR;
        } else {
            *result = MPI_UNEQUAL;
        }
    }
    
    return MPI_SUCCESS;
}

int PMPI_Type_size(MPI_Datatype datatype, int *size) {
    if (!is_mpi_initialized()) {
        return MPI_ERR_OTHER;
    }
    
    if (!size) {
        return return_or_abort(MPI_ERR_ARG);
    }

    if (datatype == MPI_DATATYPE_NULL) {
        return return_or_abort(MPI_ERR_TYPE);
    }

    if (MPILander_Type_is_builtin(datatype)) {
        auto type_size = MPILander_Type_size(datatype);
        if (type_size == -1) {
            return MPI_ERR_TYPE;
        }
        *size = type_size;
    } else {
        return return_or_abort(MPI_ERR_TYPE);
    }
    
    return MPI_SUCCESS;
}

} // extern "C"
