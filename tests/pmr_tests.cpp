#include <gtest/gtest.h>
#include <cma/cmalib.h>

TEST(cma_arena, allocate_zero_is_null) {
    cma::arena a{};
    EXPECT_EQ(a.allocate_bytes(0), nullptr);
}

TEST(cma_arena, make_construct) {
    cma::arena a{};
    int* p = a.make<int>(123);
    ASSERT_NE(p, nullptr);
    EXPECT_EQ(*p, 123);
}