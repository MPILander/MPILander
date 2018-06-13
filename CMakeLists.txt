# Preamble ####################################################################
#
cmake_minimum_required(VERSION 3.9.2)

project(MPILander VERSION 0.1.0 LANGUAGES CXX)

# the MPI standard version
# TODO parse from mpi.h
set(MPI_STANDARD_VERSION 3.1)


# Project structure ###########################################################
#
# temporary build directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    CACHE PATH "Build directory for archives")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    CACHE PATH "Build directory for libraries")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    CACHE PATH "Build directory for binaries")
# install directories
include(GNUInstallDirs)
set(CMAKE_INSTALL_CMAKEDIR "${CMAKE_INSTALL_LIBDIR}/cmake/MPILander"
    CACHE PATH "CMake config package location for installed targets")
if(WIN32)
    set(CMAKE_INSTALL_LIBDIR Lib)
    set(CMAKE_INSTALL_CMAKEDIR "cmake")
endif()


# Options and Variants ########################################################
#
set(CMAKE_CONFIGURATION_TYPES "Release;Debug;MinSizeRel;RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING
        "Choose the build type, e.g. Release or Debug." FORCE)
endif()

include(CTest)
# automatically defines: BUILD_TESTING, default is ON

option(BUILD_EXAMPLES "Build the examples" ON)


# Dependencies ################################################################
#

# none :-)

# TODO catch2


# Targets #####################################################################
#
set(HEADERS
    include/mpi.h
)
set(SOURCES
    src/mpi.cpp
)

# library
add_library(MPILander ${SOURCES} ${HEADERS})
add_library(MPILander::MPILander ALIAS MPILander)

# properties
set_target_properties(MPILander PROPERTIES OUTPUT_NAME "mpi")

target_compile_features(MPILander
    PUBLIC cxx_std_11
)
set_target_properties(MPILander PROPERTIES
    CXX_EXTENSIONS OFF
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)
if(CMAKE_BUILD_TYPE STREQUAL Debug)
    target_compile_definitions(MPILander PRIVATE "-DDEBUG")
endif()

# own headers
target_include_directories(MPILander PUBLIC
    $<BUILD_INTERFACE:${MPILander_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

# Catch2 for unit tests
#if(BUILD_TESTING)
#   TODO
#endif()

# tests
set(MPILander_TEST_NAMES
#    abc
)

# examples
#set(MPILander_EXAMPLE_NAMES
#    def
#)

if(BUILD_TESTING)
    foreach(testname ${MPILander_TEST_NAMES})
        add_executable(${testname}Tests test/${testname}.cpp)
#        if(TARGET Catch2::Catch)
#            target_link_libraries(${testname}Tests PRIVATE Catch2::Catch)
#        endif()
    endforeach()
endif()


# Generate Files with Configuration Options ###################################
#
# TODO configure a version.hpp
configure_file(
    ${MPILander_SOURCE_DIR}/MPILanderConfig.cmake.in
    ${MPILander_BINARY_DIR}/MPILanderConfig.cmake
    @ONLY
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file("MPILanderConfigVersion.cmake"
    VERSION ${MPILander_VERSION}
    COMPATIBILITY SameMajorVersion
)

# Installs ####################################################################
#
# headers, libraries and exectuables
install(TARGETS MPILander EXPORT MPILanderTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY "${MPILander_SOURCE_DIR}/include/MPILander"
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# CMake package file for find_package(MPILander::MPILander) in depending projects
install(EXPORT MPILanderTargets
    FILE MPILanderTargets.cmake
    NAMESPACE MPILander::
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)
install(
    FILES
        ${MPILander_BINARY_DIR}/MPILanderConfig.cmake
        ${MPILander_BINARY_DIR}/MPILanderConfigVersion.cmake
    DESTINATION ${CMAKE_INSTALL_CMAKEDIR}
)

# Tests #######################################################################
#
if(BUILD_TESTING)
    enable_testing()

    # C++ Unit tests
    foreach(testname ${MPILander_TEST_NAMES})
        add_test(NAME ${testname}
            COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${testname}Tests
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        )
    endforeach()
endif()
