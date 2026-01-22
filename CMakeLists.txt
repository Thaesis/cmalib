cmake_minimum_required(VERSION 3.23)
project(cma VERSION 0.1.0 LANGUAGES CXX)

add_library(cma INTERFACE)
target_include_directories(cma
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_compile_features(cma INTERFACE cxx_std_26)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(CMA_BUILD_TESTS "Build Tests" ON)
if (CMA_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()