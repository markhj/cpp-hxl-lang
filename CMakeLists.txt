cmake_minimum_required(VERSION 3.28)
project(cpp_hxl_lang)

set(CMAKE_CXX_STANDARD 20)

# Set the build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build (Debug or Release)" FORCE)
endif()

# Convert build type to lowercase
string(TOLOWER ${CMAKE_BUILD_TYPE} BUILD_TYPE_LOWER)

# Set output directories for different build configurations
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/${BUILD_TYPE_LOWER}/bin)

# Create directories (which due to subdirectories might not be possible to generate otherwise)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/build/docs)

add_library(cpp_hxl_lang STATIC
        src/traits.cpp
        src/deserializer.cpp
        src/parser.cpp
        src/processor.cpp
        src/schema-validator.cpp
        src/semantic-analyzer.cpp
        src/tokenizer.cpp
        src/transformer.cpp
        src/prfr-printer.cpp
        src/helpers.cpp)

target_include_directories(cpp_hxl_lang PUBLIC include)

# This part is primary added for some IDEs, like CLion
# to have functional inspection in header-only libraries
file(GLOB_RECURSE HEADER_FILES include/hxl-lang/*.h)
target_sources(cpp_hxl_lang INTERFACE ${HEADER_FILES})

if ($ENV{HXL_BUILD_SAMPLES})
    add_subdirectory(samples)
endif ()

# Build tests (based on C++ BBUnit)
if ($ENV{HXL_BUILD_TESTS})
    add_subdirectory(tests)
endif ()
