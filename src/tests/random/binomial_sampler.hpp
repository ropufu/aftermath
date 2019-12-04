
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../trivial_engine.hpp"
#include "../../ropufu/probability/binomial_distribution.hpp"
#include "../../ropufu/random/alias_sampler.hpp"
#include "../../ropufu/random/alias_multisampler.hpp"
#include "../../ropufu/random/binomial_sampler.hpp"

#include <array>      // std::array
#include <cstddef>    // std::size_t
#include <cstdint>    // std::int_fast32_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <stdexcept>  // std::logic_error

#define ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_ALL_TYPES                                              \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, int, float, float>,                    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, std::size_t, float, double>,           \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, std::int_fast32_t, double, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, int, float, float>,                 \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, std::size_t, float, double>,        \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, std::int_fast32_t, double, double>, \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, int, float, float>,                     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, std::size_t, float, double>,            \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, std::int_fast32_t, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, int, float, float>,                    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, std::size_t, float, double>,           \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, std::int_fast32_t, double, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, int, float, float>,                  \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, std::size_t, float, double>,         \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, std::int_fast32_t, double, double>   \


TEST_CASE_TEMPLATE("testing (randomized) binomial_sampler", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using value_type = typename tested_t::value_type;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using accurate_sampler_type = ropufu::aftermath::random::binomial_sampler<engine_type, value_type, probability_type, expectation_type>;
    using distribution_type = typename accurate_sampler_type::distribution_type;
    using alias_sampler_type = ropufu::aftermath::random::alias_sampler<engine_type, distribution_type>;

    std::string engine_name = tested_t::engine_name();
    CAPTURE(engine_name);

    engine_type engine {};
    ropufu::aftermath::tests::seed(engine);

    value_type count_trials = 64;
    distribution_type distribution_always_0 { count_trials, 0 };
    distribution_type distribution_always_1 { count_trials, 1 };
    distribution_type distribution_generic { count_trials, static_cast<probability_type>(0.1729) };

    alias_sampler_type sampler_alias_always_0 { distribution_always_0 };
    alias_sampler_type sampler_alias_always_1 { distribution_always_1 };
    alias_sampler_type sampler_alias_generic { distribution_generic };
    
    accurate_sampler_type sampler_accurate_always_0 { distribution_always_0 };
    accurate_sampler_type sampler_accurate_always_1 { distribution_always_1 };
    accurate_sampler_type sampler_accurate_generic { distribution_generic };

    std::size_t count_reps = 1'000;

    std::size_t count_alias_always_0 = 0;
    std::size_t count_alias_always_1 = 0;
    std::size_t count_alias_generic = 0;

    std::size_t count_accurate_always_0 = 0;
    std::size_t count_accurate_always_1 = 0;
    std::size_t count_accurate_generic = 0;
    for (std::size_t k = 0; k < count_reps; ++k)
    {
        count_alias_always_0 += (sampler_alias_always_0(engine));
        count_alias_always_1 += (sampler_alias_always_1(engine));
        count_alias_generic += (sampler_alias_generic(engine));

        count_accurate_always_0 += (sampler_accurate_always_0(engine));
        count_accurate_always_1 += (sampler_accurate_always_1(engine));
        count_accurate_generic += (sampler_accurate_generic(engine));
    } // for (...) 
    double estimate_p_alias = static_cast<double>(count_alias_generic) / static_cast<double>(count_trials * count_reps);
    double estimate_p_accurate = static_cast<double>(count_accurate_generic) / static_cast<double>(count_trials * count_reps);

    REQUIRE(count_alias_always_0 == 0);
    REQUIRE(count_accurate_always_0 == 0);
    REQUIRE(count_alias_always_1 == count_trials * count_reps);
    REQUIRE(count_accurate_always_1 == count_trials * count_reps);
    CHECK(estimate_p_alias == doctest::Approx(0.1729).epsilon(0.05));
    CHECK(estimate_p_accurate == doctest::Approx(0.1729).epsilon(0.05));
} // TEST_CASE_TEMPLATE(...)

TEST_CASE_TEMPLATE("testing (randomized) binomial alias_multisampler", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using value_type = typename tested_t::value_type;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using distribution_type = ropufu::aftermath::probability::binomial_distribution<value_type, probability_type, expectation_type>;
    using alias_multisampler_type = ropufu::aftermath::random::alias_multisampler<engine_type, distribution_type>;

    std::string engine_name = tested_t::engine_name();
    CAPTURE(engine_name);

    engine_type engine {};
    ropufu::aftermath::tests::seed(engine);

    value_type count_trials = 64;
    distribution_type distribution_always_0 { count_trials, 0 };
    distribution_type distribution_always_1 { count_trials, 1 };
    distribution_type distribution_generic { count_trials, static_cast<probability_type>(0.1729) };

    alias_multisampler_type multisampler {};
    multisampler.reserve(3, 3 * (count_trials + 1));
    multisampler.push_back(distribution_always_0);
    multisampler.push_back(distribution_always_1);
    multisampler.push_back(distribution_generic);
    
    std::size_t count_reps = 1'000;

    std::size_t count_always_0 = 0;
    std::size_t count_always_1 = 0;
    std::size_t count_generic = 0;
    for (std::size_t k = 0; k < count_reps; ++k)
    {
        count_always_0 += (multisampler(0, engine));
        count_always_1 += (multisampler(1, engine));
        count_generic += (multisampler(2, engine));
    } // for (...) 
    double estimate_p = static_cast<double>(count_generic) / static_cast<double>(count_trials * count_reps);

    REQUIRE(count_always_0 == 0);
    REQUIRE(count_always_1 == count_trials * count_reps);
    CHECK(estimate_p == doctest::Approx(0.1729).epsilon(0.05));
} // TEST_CASE_TEMPLATE(...)

TEST_SUITE("Benchmarks")
{
    TEST_CASE_TEMPLATE("binomial_sampler vs STL", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_ALL_TYPES)
    {
        using engine_type = typename tested_t::engine_type;
        using value_type = typename tested_t::value_type;
        using probability_type = typename tested_t::probability_type;
        using expectation_type = typename tested_t::expectation_type;
        using accurate_sampler_type = ropufu::aftermath::random::binomial_sampler<engine_type, value_type, probability_type, expectation_type>;
        using distribution_type = typename accurate_sampler_type::distribution_type;
        using builtin_sampler_type = typename distribution_type::std_type;
        
        if (!ropufu::aftermath::tests::g_do_benchmarks) return;

        std::string engine_name = tested_t::engine_name();
        CAPTURE(engine_name);

        engine_type engine {};
        ropufu::aftermath::tests::seed(engine);

        value_type count_trials = 64;
        distribution_type distribution { count_trials, static_cast<probability_type>(0.1729) };
        accurate_sampler_type sampler { distribution };
        builtin_sampler_type builtin_sampler = distribution.to_std();

        constexpr std::size_t sample_size = 1'000'000 / ropufu::aftermath::tests::template engine_slowdown_factor<engine_type>(10);

        double seconds_fast = ropufu::aftermath::tests::sample_timing(sample_size, engine, sampler);
        double seconds_slow = ropufu::aftermath::tests::sample_timing(sample_size, engine, builtin_sampler);

        BENCH_COMPARE_TIMING(engine_name, "aftermath", "builtin", seconds_fast, seconds_slow);
    } // TEST_CASE_TEMPLATE(...)

    TEST_CASE_TEMPLATE("binomial_sampler alias vs accurate", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_ALL_TYPES)
    {
        using engine_type = typename tested_t::engine_type;
        using value_type = typename tested_t::value_type;
        using probability_type = typename tested_t::probability_type;
        using expectation_type = typename tested_t::expectation_type;
        using accurate_sampler_type = ropufu::aftermath::random::binomial_sampler<engine_type, value_type, probability_type, expectation_type>;
        using distribution_type = typename accurate_sampler_type::distribution_type;
        using alias_sampler_type = ropufu::aftermath::random::alias_sampler<engine_type, distribution_type>;
        
        if (!ropufu::aftermath::tests::g_do_benchmarks) return;

        std::string engine_name = tested_t::engine_name();
        CAPTURE(engine_name);

        engine_type engine {};
        ropufu::aftermath::tests::seed(engine);

        value_type count_trials = 64;
        distribution_type distribution { count_trials, static_cast<probability_type>(0.1729) };
        alias_sampler_type alias_sampler { distribution };
        accurate_sampler_type accurate_sampler { distribution };

        constexpr std::size_t sample_size = 1'000'000 / ropufu::aftermath::tests::template engine_slowdown_factor<engine_type>();

        double seconds_fast = ropufu::aftermath::tests::sample_timing(sample_size, engine, alias_sampler);
        double seconds_slow = ropufu::aftermath::tests::sample_timing(sample_size, engine, accurate_sampler);

        BENCH_COMPARE_TIMING(engine_name, "alias", "accurate", seconds_fast, seconds_slow);
    } // TEST_CASE_TEMPLATE(...)
} // TEST_SUITE(..)

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_BINOMIAL_SAMPLER_HPP_INCLUDED
