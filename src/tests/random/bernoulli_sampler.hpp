
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../trivial_engine.hpp"
#include "../../ropufu/probability/bernoulli_distribution.hpp"
#include "../../ropufu/random/bernoulli_sampler.hpp"

#include <array>      // std::array
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_ALL_TYPES                                 \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, bool, float, float>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, bool, float, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, bool, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, bool, float, float>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, bool, float, double>,   \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, bool, double, double>,  \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, bool, float, float>,        \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, bool, float, double>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, bool, double, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, bool, float, float>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, bool, float, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, bool, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, bool, float, float>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, bool, float, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, bool, double, double>    \


TEST_CASE_TEMPLATE("testing (randomized) bernoulli_sampler", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using sampler_type = ropufu::aftermath::random::bernoulli_sampler<engine_type, probability_type, expectation_type>;
    using distribution_type = typename sampler_type::distribution_type;

    std::string engine_name = tested_t::engine_name();
    CAPTURE(engine_name);

    engine_type engine {};
    ropufu::aftermath::tests::seed(engine);

    distribution_type distribution_always_0 { 0 };
    distribution_type distribution_always_1 { 1 };
    distribution_type distribution_generic { static_cast<probability_type>(0.1729) };

    sampler_type sampler_always_0 { distribution_always_0 };
    sampler_type sampler_always_1 { distribution_always_1 };
    sampler_type sampler_generic { distribution_generic };

    std::size_t count_reps = 10'000;
    std::size_t count_always_0 = 0;
    std::size_t count_always_1 = 0;
    std::size_t count_generic = 0;
    for (std::size_t k = 0; k < count_reps; ++k)
    {
        if (sampler_always_0(engine)) ++count_always_0;
        if (sampler_always_1(engine)) ++count_always_1;
        if (sampler_generic(engine)) ++count_generic;
    } // for (...) 
    double estimate_p = static_cast<double>(count_generic) / static_cast<double>(count_reps);

    REQUIRE(count_always_0 == 0);
    REQUIRE(count_always_1 == count_reps);
    CHECK(estimate_p == doctest::Approx(0.1729).epsilon(0.05));
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing (trivial) bernoulli_sampler", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using engine_min_type = ropufu::aftermath::tests::trivial_engine<
        typename engine_type::result_type,
        engine_type::min(), engine_type::max(), engine_type::min()>;
    using engine_max_type = ropufu::aftermath::tests::trivial_engine<
        typename engine_type::result_type,
        engine_type::min(), engine_type::max(), engine_type::max()>;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using sampler_min_type = ropufu::aftermath::random::bernoulli_sampler<engine_min_type, probability_type, expectation_type>;
    using sampler_max_type = ropufu::aftermath::random::bernoulli_sampler<engine_max_type, probability_type, expectation_type>;
    using distribution_type = ropufu::aftermath::probability::bernoulli_distribution<probability_type, expectation_type>;

    std::string engine_name = tested_t::engine_name();
    CAPTURE(engine_name);
    
    engine_min_type engine_min {};
    engine_max_type engine_max {};

    distribution_type distribution_always_0 { 0 };
    distribution_type distribution_always_1 { 1 };
    distribution_type distribution_generic { static_cast<probability_type>(0.1729) };

    sampler_max_type sampler_always_0 { distribution_always_0 };
    sampler_min_type sampler_always_1 { distribution_always_1 };
    sampler_min_type sampler_generic_min { distribution_generic };
    sampler_max_type sampler_generic_max { distribution_generic };

    std::size_t count_reps = 100;
    std::size_t count_always_0 = 0;
    std::size_t count_always_1 = 0;
    std::size_t count_generic_min = 0;
    std::size_t count_generic_max = 0;
    for (std::size_t k = 0; k < count_reps; ++k)
    {
        if (sampler_always_0(engine_max)) ++count_always_0;
        if (sampler_always_1(engine_min)) ++count_always_1;
        if (sampler_generic_min(engine_min)) ++count_generic_min;
        if (sampler_generic_max(engine_max)) ++count_generic_max;
    } // for (...) 

    REQUIRE(count_always_0 == 0);
    REQUIRE(count_always_1 == count_reps);

    REQUIRE(count_generic_min == count_reps);
    REQUIRE(count_generic_max == 0);
} // TEST_CASE_TEMPLATE(...)

TEST_SUITE("Benchmarks")
{
    TEST_CASE_TEMPLATE("bernoulli_sampler vs STL", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_ALL_TYPES)
    {
        using engine_type = typename tested_t::engine_type;
        using probability_type = typename tested_t::probability_type;
        using expectation_type = typename tested_t::expectation_type;
        using sampler_type = ropufu::aftermath::random::bernoulli_sampler<engine_type, probability_type, expectation_type>;
        using distribution_type = typename sampler_type::distribution_type;
        using builtin_sampler_type = typename distribution_type::std_type;
        
        if (!ropufu::aftermath::tests::g_do_benchmarks) return;

        std::string engine_name = tested_t::engine_name();
        CAPTURE(engine_name);

        engine_type engine {};
        ropufu::aftermath::tests::seed(engine);

        distribution_type distribution { static_cast<probability_type>(0.1729) };
        sampler_type sampler { distribution };
        builtin_sampler_type builtin_sampler = distribution.to_std();

        constexpr std::size_t sample_size = 10'000'000 / ropufu::aftermath::tests::template engine_slowdown_factor<engine_type>(10);

        double seconds_fast = ropufu::aftermath::tests::sample_timing(sample_size, engine, sampler);
        double seconds_slow = ropufu::aftermath::tests::sample_timing(sample_size, engine, builtin_sampler);

        BENCH_COMPARE_TIMING(engine_name, "aftermath", "builtin", seconds_fast, seconds_slow);
    } // TEST_CASE_TEMPLATE(...)
} // TEST_SUITE(..)

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_BERNOULLI_SAMPLER_HPP_INCLUDED
