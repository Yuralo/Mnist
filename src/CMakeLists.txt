cmake_minimum_required(VERSION 3.12)
project(MNIST VERSION 1.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set build type if not specified
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -ffast-math")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# Find Accelerate framework on macOS
if(APPLE)
    find_library(ACCELERATE_LIBRARY Accelerate)
    if(ACCELERATE_LIBRARY)
        message(STATUS "Found Accelerate framework")
    else()
        message(WARNING "Accelerate framework not found")
    endif()
endif()

# Source files
set(DLARRAY_SOURCES dlarray.cpp)
set(TERMINAL_PLOT_SOURCES terminal_plot.cpp)
set(MNIST_SOURCES mnist.cpp)

# Executable: MNIST
add_executable(mnist
    ${MNIST_SOURCES}
    ${DLARRAY_SOURCES}
    ${TERMINAL_PLOT_SOURCES}
)

# Link Accelerate framework on macOS
if(APPLE AND ACCELERATE_LIBRARY)
    target_link_libraries(mnist ${ACCELERATE_LIBRARY})
endif()

# Include directories
target_include_directories(mnist PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(plot_example
    plot_example.cpp
    ${DLARRAY_SOURCES}
    ${TERMINAL_PLOT_SOURCES}
)

if(APPLE AND ACCELERATE_LIBRARY)
    target_link_libraries(plot_example ${ACCELERATE_LIBRARY})
endif()

target_include_directories(plot_example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

