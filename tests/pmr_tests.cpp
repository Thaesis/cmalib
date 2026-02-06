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

TEST(cma_pmr_adaptor, vector_adaptor) {
    cma::arena a{};
    cma::cma_resource r{a};
    std::pmr::vector<int> v{ &r };
    v.push_back(1);
    EXPECT_EQ(v.front(), 1);
    v.pop_back();
    EXPECT_EQ(v.empty(), true);
}
