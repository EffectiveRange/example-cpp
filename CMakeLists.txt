cmake_minimum_required(VERSION 3.22)

set(CMAKE_COMPILE_WARNING_AS_ERROR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pull in definitions from
# Effective Range CMake module like ER_DEPS, ER_PACK, etc.
include(ERBuild NO_POLICY_SCOPE)

set(sample_proj_ver 0.1.0)

project(er_example_cpp VERSION ${sample_proj_ver} DESCRIPTION "This is a sample project demonstrating the dev tooling capabilities")



# GSS_DEPS() should be called once per top level project, this
# will install the package dependencies listed in the deps.json file 
ER_DEPS()

# Lookup dependencies and make them available to CMake
find_package(fmt REQUIRED)
find_package(Catch2  REQUIRED)
# This is a thin wrapper on top of vanilla cmake add_executable
# it just marks the artifacts for installation automatically
# so the binaries will be packaged without any other user interaction
ER_ADD_EXECUTABLE(er_example_cpp SOURCES src/main.cpp)

# add gpio directory to the build
add_subdirectory(gpio)

# cross platform wrapper, to only enable running tests when the host arch == target arch
ER_ENABLE_TEST()

add_subdirectory(test)
# this function performs packaging into a debian package, all metadata is 
# extracted from the project description and the deps.json file
ER_PACK()