
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/stopping_time.hpp"

#include <cstddef> // std::size_t
#include <cstdint> // std::int64_t
#include <random>  // std::mt19937_64
#include <string>  // std::string
#include <vector>  // std::vector

#ifdef ROPUFU_TMP_TEST_TYPES
#undef ROPUFU_TMP_TEST_TYPES
#endif
#define ROPUFU_TMP_TEST_TYPES                                                      \
    ropufu::aftermath::random::binomial_sampler<std::mt19937_64, std::int64_t>,    \
    ropufu::aftermath::random::normal_sampler_512<std::mt19937_64, double>,        \
    ropufu::aftermath::random::uniform_int_sampler<std::mt19937_64, std::int64_t>  \


#ifndef ROPUFU_NO_JSON
TEST_CASE_TEMPLATE("testing stopping_time json", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using stopping_time_type = ropufu::aftermath::sequential::stopping_time<value_type>;
    using container_type = typename stopping_time_type::container_type;
    
    stopping_time_type stopping_time_o{};
    stopping_time_type stopping_time_a{container_type({3})};
    stopping_time_type stopping_time_b{container_type({1, 2, 5})};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(stopping_time_o, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(stopping_time_a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(stopping_time_b, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing stopping_time border crossing", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using stopping_time_type = ropufu::aftermath::sequential::stopping_time<value_type>;
    using container_type = typename stopping_time_type::container_type;
    
    stopping_time_type stopping_time{container_type({1, 2, 5})};

    std::vector<value_type> process = {0, -1, 1, 2, 0, 3, 3};
    // ======================================================
    // Time:                           1,  2, 3, 4, 5, 6, 7
    // ======================================================
    // First value > 1:                          ^
    // First value > 2:                                ^
    // First value > 5:                never
    // ======================================================
    for (value_type x : process)
    {
        stopping_time.observe(x);
    } // for (...)

    CHECK_EQ(stopping_time.is_running(), true); // First process never crosses 5.
    CHECK_EQ(stopping_time.when(0), 4); // First process > 1.
    CHECK_EQ(stopping_time.when(1), 6); // First process > 2.
    CHECK_EQ(stopping_time.when(2), 0); // First process > 5.
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
