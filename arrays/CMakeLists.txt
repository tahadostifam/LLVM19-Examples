cmake_minimum_required(VERSION 3.28) 

project(llvm_examples)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic") 

# Find LLVM
find_package(LLVM REQUIRED CONFIG)
if(LLVM_FOUND)
    message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
    message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")
else()
    message(FATAL_ERROR "LLVM is required to build this project.")
endif()

# Find ZLIB
find_package(ZLIB REQUIRED) 
if(ZLIB)
  message(STATUS "Found ZLIB: ${ZLIB_EXECUTABLE}")
endif()

# Define source directories
set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/src)
set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)

include_directories(
    ${INCLUDE_DIR}
    ${SOURCE_DIR}  
)

# Collect all source files.
file(GLOB_RECURSE source_files
    ${SOURCE_DIR}/*.cpp
)
set(source_files ${source_files} ${set_source_files})

# Create a Library
add_library(llvm_examples ${source_files})

# Link LLVM Libraries
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})
llvm_map_components_to_libnames(llvm_libs support core irreader)

# Add third party libraries
include(FetchContent)
FetchContent_Declare(nlohmann_json URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz)
FetchContent_MakeAvailable(nlohmann_json)
target_link_libraries(llvm_examples nlohmann_json::nlohmann_json)
include_directories(${nlohmann_json_SOURCE_DIR}/single_include/) 

# Create an executable
add_executable(llvm_examples_array 
    ${SOURCE_DIR}/main.cpp
) 
target_link_libraries(llvm_examples_array llvm_examples ${llvm_libs})