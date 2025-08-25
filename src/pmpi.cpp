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

// MPI profiling interface - MPI functions forward to PMPI functions
// This allows profiling tools to intercept MPI calls by providing their own MPI_* functions

extern "C" {

// Forward declarations for PMPI functions (implemented in mpi.cpp)
int PMPI_Init(int *argc, char ***argv);
int PMPI_Finalize();
int PMPI_Comm_rank(MPI_Comm comm, int *rank);
int PMPI_Comm_size(MPI_Comm comm, int *size);
int PMPI_Abort(MPI_Comm comm, int errorcode);
int PMPI_Initialized(int *flag);
int PMPI_Finalized(int *flag);
int PMPI_Init_thread(int *argc, char ***argv, int required, int *provided);
int PMPI_Query_thread(int *provided);
int PMPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm);
int PMPI_Comm_free(MPI_Comm *comm);
int PMPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result);
int PMPI_Type_size(MPI_Datatype datatype, int *size);

// MPI functions forward to PMPI functions using weak symbols or direct forwarding
#if defined(HAVE_PRAGMA_WEAK)
#pragma weak MPI_Init = PMPI_Init
#pragma weak MPI_Finalize = PMPI_Finalize
#pragma weak MPI_Comm_rank = PMPI_Comm_rank
#pragma weak MPI_Comm_size = PMPI_Comm_size
#pragma weak MPI_Abort = PMPI_Abort
#pragma weak MPI_Initialized = PMPI_Initialized
#pragma weak MPI_Finalized = PMPI_Finalized
#pragma weak MPI_Init_thread = PMPI_Init_thread
#pragma weak MPI_Query_thread = PMPI_Query_thread
#pragma weak MPI_Comm_dup = PMPI_Comm_dup
#pragma weak MPI_Comm_free = PMPI_Comm_free
#pragma weak MPI_Comm_compare = PMPI_Comm_compare
#pragma weak MPI_Type_size = PMPI_Type_size
#elif defined(HAVE_WEAK_ATTRIBUTE)
int MPI_Init(int *argc, char ***argv)
    __attribute__ ((weak, alias("PMPI_Init")));
int MPI_Finalize()
    __attribute__ ((weak, alias("PMPI_Finalize")));
int MPI_Comm_rank(MPI_Comm comm, int *rank)
    __attribute__ ((weak, alias("PMPI_Comm_rank")));
int MPI_Comm_size(MPI_Comm comm, int *size)
    __attribute__ ((weak, alias("PMPI_Comm_size")));
int MPI_Abort(MPI_Comm comm, int errorcode)
    __attribute__ ((weak, alias("PMPI_Abort")));
int MPI_Initialized(int *flag)
    __attribute__ ((weak, alias("PMPI_Initialized")));
int MPI_Finalized(int *flag)
    __attribute__ ((weak, alias("PMPI_Finalized")));
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
    __attribute__ ((weak, alias("PMPI_Init_thread")));
int MPI_Query_thread(int *provided)
    __attribute__ ((weak, alias("PMPI_Query_thread")));
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm)
    __attribute__ ((weak, alias("PMPI_Comm_dup")));
int MPI_Comm_free(MPI_Comm *comm)
    __attribute__ ((weak, alias("PMPI_Comm_free")));
int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result)
    __attribute__ ((weak, alias("PMPI_Comm_compare")));
int MPI_Type_size(MPI_Datatype datatype, int *size)
    __attribute__ ((weak, alias("PMPI_Type_size")));
#else
// Fallback: direct function forwarding
int MPI_Init(int *argc, char ***argv) { return PMPI_Init(argc, argv); }
int MPI_Finalize() { return PMPI_Finalize(); }
int MPI_Comm_rank(MPI_Comm comm, int *rank) { return PMPI_Comm_rank(comm, rank); }
int MPI_Comm_size(MPI_Comm comm, int *size) { return PMPI_Comm_size(comm, size); }
int MPI_Abort(MPI_Comm comm, int errorcode) { return PMPI_Abort(comm, errorcode); }
int MPI_Initialized(int *flag) { return PMPI_Initialized(flag); }
int MPI_Finalized(int *flag) { return PMPI_Finalized(flag); }
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided) { return PMPI_Init_thread(argc, argv, required, provided); }
int MPI_Query_thread(int *provided) { return PMPI_Query_thread(provided); }
int MPI_Comm_dup(MPI_Comm comm, MPI_Comm *newcomm) { return PMPI_Comm_dup(comm, newcomm); }
int MPI_Comm_free(MPI_Comm *comm) { return PMPI_Comm_free(comm); }
int MPI_Comm_compare(MPI_Comm comm1, MPI_Comm comm2, int *result) { return PMPI_Comm_compare(comm1, comm2, result); }
int MPI_Type_size(MPI_Datatype datatype, int *size) { return PMPI_Type_size(datatype, size); }
#endif

} // extern "C"
