
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

namespace ropufu::tests
{
    template <typename t_value_type>
    struct stopped_statistic_for_stopping_time
    {
        using value_type = t_value_type;

        value_type operator ()(std::size_t time) const noexcept
        {
            return static_cast<value_type>(time);
        } // operator (...)
    }; // struct stopped_statistic_for_stopping_time
} // namespace ropufu::tests

#ifdef ROPUFU_TMP_TEST_TYPES
#undef ROPUFU_TMP_TEST_TYPES
#endif
#define ROPUFU_TMP_TEST_TYPES std::int64_t, float, double

TEST_CASE_TEMPLATE("testing stopping_time border crossing", value_type, ROPUFU_TMP_TEST_TYPES)
{
    using stopping_time_type = ropufu::aftermath::sequential::stopping_time<value_type>;
    
    std::vector<value_type> thresholds{1, 2, 5};
    stopping_time_type stopping_time{thresholds};

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

TEST_CASE_TEMPLATE("testing stopping_time stopped_statistic", value_type, ROPUFU_TMP_TEST_TYPES)
{
    using stopped_statistic_type = ropufu::tests::stopped_statistic_for_stopping_time<std::size_t>;
    using stopping_time_type = ropufu::aftermath::sequential::stopping_time<value_type, stopped_statistic_type>;
    
    std::vector<value_type> thresholds{1, 2, 5};
    stopping_time_type stopping_time{thresholds};

    std::vector<value_type> process = {0, -1, 1, 2, 0, 3, 3, 10};
    for (value_type x : process) stopping_time.observe(x);

    REQUIRE_EQ(stopping_time.is_running(), false);
    CHECK_EQ(stopping_time.stopped_statistic(), stopping_time.when());
} // TEST_CASE_TEMPLATE(...)

#endif // ROPUFU_AFTERMATH_TESTS_SEQUENTIAL_STOPPING_TIME_HPP_INCLUDED
