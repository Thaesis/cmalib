include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

# Prevents gtest from overriding compiler flags
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

add_executable(cma_tests
    pmr_tests.cpp
)

target_link_libraries(cma_tests
    PRIVATE
        cma
        GTest::gtest_main
)

target_compile_features(cma_tests PRIVATE cxx_std_26)

include(GoogleTest)
gtest_discover_tests(cma_tests)