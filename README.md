# MPILander

There can only be one (MPI process)!

MPILander is a single-rank [mock library](https://en.wikipedia.org/wiki/Mock_object).
It aims to implement a full MPI 3.1 standard compatible MPI library with the most trivial program path: allowing *only one MPI process (rank)*!


## Why would I need MPILander?

MPILander is perfect for massively parallel applications and libraries that *also run well in single-rank mode*.
Often one faces the problem that either the main application or its dependencies are heavily relying on MPI functionality.
This complicates starting with one rank (e.g., still needs to call `mpiexec`) in many environments such as [CI](https://en.wikipedia.org/wiki/Continuous_integration) or intentionally serial (debug) workflows.

MPILander is here to help you with that.
Keep your regular MPI hard-wiring in place - and it will implement and provide everything you would expect from a MPI library, but only for exactly one and only one rank.


## Dependencies

Required:
* CMake 3.9.2+
* C++11 capable compiler, e.g. g++ 4.8+, clang 3.9+, VS 2015+


## Installation

### [Spack](https://spack.io)

*TBD*

### From Source

MPILander can be installed using [CMake](http://cmake.org/):

```bash
git clone https://github.com/jeffhammond/MPILander.git

mkdir -p MPILander-api-build
cd MPILander-api-build

# for own install prefix append:
#   -DCMAKE_INSTALL_PREFIX=$HOME/somepath
cmake ../MPILander

cmake --build .

# optional
ctest

# sudo might be required for system paths
cmake --build . --target install
```

By default, this will build as a static library (`libMPILander.a`) and installs also its headers.
In order to build a static library, append `-DBUILD_SHARED_LIBS=ON` to the `cmake` command.
You can only build a static or a shared library at a time.

By default, the `Release` version is built.
In order to build with debug symbols, pass `-DCMAKE_BUILD_TYPE=Debug` to your `cmake` command.

By default, tests and examples are built.
In order to skip building those, pass `-DBUILD_TESTING=OFF` or `-DBUILD_EXAMPLES` to your `cmake` command.


## Using MPILander

### CMake

If your project is using CMake for its build, one can conveniently use our provided `Config.cmake` package which is installed alongside the library.

First set the following environment hint if MPILander was *not* installed in a system path:

```bash
# optional: only needed if installed outside of system paths
export CMAKE_PREFIX_PATH=$HOME/somepath:$CMAKE_PREFIX_PATH
```

Use the following lines in your projects `CMakeLists.txt`:
```cmake
find_package(MPILander 0.1.0 CONFIG)

if(MPILander_FOUND)
    target_link_libraries(YourTarget PRIVATE MPILander::MPILander)
endif()
```

*Alternatively*, add the MPILander repository directly to your project and add it via

```cmake
add_subdirectory("path/to/source/of/MPILander")

target_link_libraries(YourTarget PRIVATE MPILander::MPILander)
```
