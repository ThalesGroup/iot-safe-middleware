# Minimum version of CMake 3.14.5
cmake_minimum_required (VERSION 3.14.5)
# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)
enable_testing()
project (IOTSAFE)
add_subdirectory (iotsafelib)
add_subdirectory (external_libs)
add_subdirectory (tests)
add_subdirectory (example)
