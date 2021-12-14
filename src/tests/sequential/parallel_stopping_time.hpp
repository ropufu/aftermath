
#ifndef ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED

#include <doctest/doctest.h>

#include "../core.hpp"
#include "../../ropufu/algebra/matrix.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"
#include "../../ropufu/sequential/parallel_stopping_time.hpp"

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
TEST_CASE_TEMPLATE("testing parallel_stopping_time json", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using parallel_stopping_time_type = ropufu::aftermath::sequential::parallel_stopping_time<value_type>;
    using scalar_container_type = typename parallel_stopping_time_type::scalar_container_type;
    
    parallel_stopping_time_type parallel_stopping_time_o{};
    parallel_stopping_time_type parallel_stopping_time_a{scalar_container_type({3}), scalar_container_type({1, 2, 5})};
    parallel_stopping_time_type parallel_stopping_time_b{scalar_container_type({1, 2, 5}), scalar_container_type({4})};

    std::string xxx {};
    std::string yyy {};

    ropufu::tests::does_json_round_trip(parallel_stopping_time_o, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(parallel_stopping_time_a, xxx, yyy);
    CHECK_EQ(xxx, yyy);

    ropufu::tests::does_json_round_trip(parallel_stopping_time_b, xxx, yyy);
    CHECK_EQ(xxx, yyy);
} // TEST_CASE_TEMPLATE(...)
#endif

TEST_CASE_TEMPLATE("testing parallel_stopping_time border crossing", sampler_type, ROPUFU_TMP_TEST_TYPES)
{
    using value_type = typename sampler_type::value_type;
    using parallel_stopping_time_type = ropufu::aftermath::sequential::parallel_stopping_time<value_type>;
    using scalar_container_type = typename parallel_stopping_time_type::scalar_container_type;


    // ========================================================
    //   (1, 0) -- (1, 4)
    //   (2, 0) -- (2, 4)
    //   (5, 0) -- (5, 4)
    // ========================================================
    parallel_stopping_time_type parallel_stopping_time{
        scalar_container_type({1, 2, 5}),
        scalar_container_type({4, 0})};
    // std::size_t m = parallel_stopping_time.vertical_thresholds().size();
    // std::size_t n = parallel_stopping_time.horizontal_thresholds().size();

    std::vector<value_type> process_a = {0, -1, 1, 2, 0, 3, 3};
    // ========================================================
    // Time:                             1,  2, 3, 4, 5, 6, 7
    // ========================================================
    // First value > 1:                            ^
    // First value > 2:                                  ^
    // First value > 5:                  never
    // ========================================================
    std::vector<value_type> process_b = {1, 4, -2, 3, 0, 7, 0};
    // ========================================================
    // Time:                             1, 2,  3, 4, 5, 6, 7
    // ========================================================
    // Second value > 0:                 ^
    // Second value > 4:                                 ^
    // ========================================================
    REQUIRE_EQ(process_a.size(), process_b.size());
    
    ropufu::aftermath::algebra::matrix<std::size_t> reference_when = {
        {1, 4},
        {1, 6},
        {1, 6}
    };
    ropufu::aftermath::algebra::matrix<char> reference_which = {
        {2, 1},
        {2, 3},
        {2, 2}
    };

    for (std::size_t i = 0; i < process_a.size(); ++i)
    {
        parallel_stopping_time.observe(std::make_pair(process_a[i], process_b[i]));
    } // for (...)

    CHECK_EQ(parallel_stopping_time.is_running(), false); // Second process crosses all thresholds.
    CHECK_EQ(parallel_stopping_time.when(), reference_when);
    CHECK_EQ(parallel_stopping_time.which(), reference_which);
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_PARALLEL_STOPPING_TIME_HPP_INCLUDED
