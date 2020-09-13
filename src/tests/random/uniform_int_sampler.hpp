
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../../ropufu/probability/uniform_int_distribution.hpp"
#include "../../ropufu/random/uniform_int_sampler.hpp"

#include <array>      // std::array
#include <cstddef>    // std::size_t
#include <cstdint>    // std::int_fast32_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_ALL_TYPES                                           \
    ropufu::tests::engine_distribution_tuple<std::ranlux24, int, float, float>,                    \
    ropufu::tests::engine_distribution_tuple<std::ranlux24, std::size_t, float, double>,           \
    ropufu::tests::engine_distribution_tuple<std::ranlux24, std::int_fast32_t, double, double>,    \
    ropufu::tests::engine_distribution_tuple<std::minstd_rand, int, float, float>,                 \
    ropufu::tests::engine_distribution_tuple<std::minstd_rand, std::size_t, float, double>,        \
    ropufu::tests::engine_distribution_tuple<std::minstd_rand, std::int_fast32_t, double, double>, \
    ropufu::tests::engine_distribution_tuple<std::mt19937, int, float, float>,                     \
    ropufu::tests::engine_distribution_tuple<std::mt19937, std::size_t, float, double>,            \
    ropufu::tests::engine_distribution_tuple<std::mt19937, std::int_fast32_t, double, double>,     \
    ropufu::tests::engine_distribution_tuple<std::ranlux48, int, float, float>,                    \
    ropufu::tests::engine_distribution_tuple<std::ranlux48, std::size_t, float, double>,           \
    ropufu::tests::engine_distribution_tuple<std::ranlux48, std::int_fast32_t, double, double>,    \
    ropufu::tests::engine_distribution_tuple<std::mt19937_64, int, float, float>,                  \
    ropufu::tests::engine_distribution_tuple<std::mt19937_64, std::size_t, float, double>,         \
    ropufu::tests::engine_distribution_tuple<std::mt19937_64, std::int_fast32_t, double, double>   \


TEST_CASE_TEMPLATE("testing (randomized) uniform_int_sampler", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using value_type = typename tested_t::value_type;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using sampler_type = ropufu::aftermath::random::uniform_int_sampler<engine_type, value_type, probability_type, expectation_type>;
    using distribution_type = typename sampler_type::distribution_type;

    value_type a = 2;
    value_type b = 8;
    double midpoint = static_cast<double>(a + b) / 2;

    std::string engine_name {tested_t::engine_name()};
    CAPTURE(engine_name);

    engine_type engine {};
    ropufu::tests::seed(engine);

    distribution_type distribution_always_a { a, a };
    distribution_type distribution_always_b { b, b };
    distribution_type distribution_a_b { a, b };

    sampler_type sampler_always_a { distribution_always_a };
    sampler_type sampler_always_b { distribution_always_b };
    sampler_type sampler_a_b { distribution_a_b };

    std::size_t count_reps = 10'000;
    double sum_always_a = 0;
    double sum_always_b = 0;
    double sum_a_b = 0;
    for (std::size_t k = 0; k < count_reps; ++k)
    {
        sum_always_a += sampler_always_a(engine);
        sum_always_b += sampler_always_b(engine);
        sum_a_b += sampler_a_b(engine);
    } // for (...) 
    double estimate_midpoint = sum_a_b / static_cast<double>(count_reps);

    REQUIRE(sum_always_a == a * count_reps);
    REQUIRE(sum_always_b == b * count_reps);
    REQUIRE(sum_a_b >= sum_always_a);
    REQUIRE(sum_a_b <= sum_always_b);
    CHECK(estimate_midpoint == doctest::Approx(midpoint).epsilon(0.05));
} // TEST_CASE_TEMPLATE(...)

TEST_SUITE("Benchmarks")
{
    TEST_CASE_TEMPLATE("uniform_int_sampler vs STL", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_ALL_TYPES)
    {
        using engine_type = typename tested_t::engine_type;
        using value_type = typename tested_t::value_type;
        using probability_type = typename tested_t::probability_type;
        using expectation_type = typename tested_t::expectation_type;
        using sampler_type = ropufu::aftermath::random::uniform_int_sampler<engine_type, value_type, probability_type, expectation_type>;
        using distribution_type = typename sampler_type::distribution_type;
        using builtin_sampler_type = typename distribution_type::std_type;
        
        if (!ropufu::tests::g_do_benchmarks) return;

        std::string engine_name {tested_t::engine_name()};
        CAPTURE(engine_name);

        engine_type engine {};
        ropufu::tests::seed(engine);

        distribution_type distribution { 100, 1729 };
        sampler_type sampler { distribution };
        builtin_sampler_type builtin_sampler = distribution.to_std();

        constexpr std::size_t sample_size = 10'000'000 / ropufu::tests::template engine_slowdown_factor<engine_type>(5);

        double seconds_fast = ropufu::tests::sample_timing(sample_size, engine, sampler);
        double seconds_slow = ropufu::tests::sample_timing(sample_size, engine, builtin_sampler);

        BENCH_COMPARE_TIMING(engine_name, "aftermath", "builtin", seconds_fast, seconds_slow);
    } // TEST_CASE_TEMPLATE(...)
} // TEST_SUITE(..)

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_UNIFORM_INT_SAMPLER_HPP_INCLUDED
