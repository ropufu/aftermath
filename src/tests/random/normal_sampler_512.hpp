
#ifndef ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
#define ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED

#include <doctest/doctest.h>
#include "../benchmark_reporter.hpp"

#include "../core.hpp"
#include "../../ropufu/probability/standard_normal_distribution.hpp"
#include "../../ropufu/random/normal_sampler_512.hpp"

#include <array>      // std::array
#include <cmath>      // std::log
#include <cstddef>    // std::size_t
#include <limits>     // std::numeric_limits
#include <random>     // std::mt19937, ...
#include <set>        // std::multiset
#include <stdexcept>  // std::logic_error

namespace ropufu::aftermath::tests
{
    template <typename t_engine_type, typename t_sampler_type>
    void timing_vs_builtin(std::size_t sample_size, double& aftermath_ms, double& builtin_ms) noexcept
    {
        using distribution_type = typename t_sampler_type::distribution_type;

        t_engine_type engine {};
        ropufu::aftermath::tests::seed(engine);

        aftermath_ms = ropufu::aftermath::tests::benchmark(
            [&engine, sample_size] () {
                long double sum = 0;
                t_sampler_type sampler {};
                for (std::size_t k = 0; k < sample_size; ++k) sum += sampler(engine);
            });

        builtin_ms = ropufu::aftermath::tests::benchmark(
            [&engine, sample_size] () {
                long double sum = 0;
                typename distribution_type::std_type distribution {};
                for (std::size_t k = 0; k < sample_size; ++k) sum += distribution(engine);
            });
    } // timing_vs_builtin(...)
} // namespace ropufu::aftermath::tests

#define ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_ALL_TYPES                                 \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, float, float, float>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, float, float, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, float, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux24, double, double, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, float, float, float>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, float, float, double>,   \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, float, double, double>,  \
    ropufu::aftermath::tests::engine_distribution_tuple<std::minstd_rand, double, double, double>, \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, float, float, float>,        \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, float, float, double>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, float, double, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937, double, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, double, double, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, float, float, float>,       \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, float, float, double>,      \
    ropufu::aftermath::tests::engine_distribution_tuple<std::ranlux48, float, double, double>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, float, float, float>,     \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, float, float, double>,    \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, float, double, double>,   \
    ropufu::aftermath::tests::engine_distribution_tuple<std::mt19937_64, double, double, double>   \


TEST_CASE_TEMPLATE("testing (randomized) normal_sampler_512 Anderson-Darling", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_ALL_TYPES)
{
    using engine_type = typename tested_t::engine_type;
    using value_type = typename tested_t::value_type;
    using probability_type = typename tested_t::probability_type;
    using expectation_type = typename tested_t::expectation_type;
    using sampler_type = ropufu::aftermath::random::normal_sampler_512<engine_type, value_type, probability_type, expectation_type>;
    using distribution_type = typename sampler_type::distribution_type;

    engine_type engine {};
    ropufu::aftermath::tests::seed(engine);

    sampler_type sampler {};
    distribution_type distribution {};

    double count_reps = 16;
    double count_bad = 0;
    
    // constexpr long double significance_0_10_threshold = 1.9329578327415937304L; // P(at least 1 of 16 fails) = 81.5%.
    // constexpr long double significance_0_05_threshold = 2.4923671600494096176L; // P(at least 1 of 16 fails) = 56.0%.
    constexpr long double significance_0_01_threshold = 3.8781250216053948842L; // P(at least 1 of 16 fails) = 14.9%.

    // constexpr double chance_of_failure_0_10 = 0.815;
    // constexpr double chance_of_failure_0_05 = 0.560;
    constexpr double chance_of_failure_0_01 = 0.149;

    for (std::size_t k = 0; k < count_reps; ++k)
    {
        // Initialize the sample.
        constexpr std::size_t sample_size = 1024;
        std::multiset<value_type> sample {};
        for (std::size_t i = 0; i < sample_size; ++i) sample.insert(sampler(engine));

        // Calculate Anderson-Darling statistic.
        long double a = 0;
        std::size_t index = 1;
        for (const value_type& x : sample)
        {
            a += (2 * index - 1) * std::log(distribution.cdf(x));
            a += (2 * (sample_size - index) + 1) * std::log(1 - distribution.cdf(x));
            ++index;
        } // for (...)
        a = -static_cast<long double>(sample_size) - (a / sample_size);
        
        //CHECK(a < significance_0_01_threshold);
        if (a >= significance_0_01_threshold) ++count_bad;
    } // for (...) 

     CHECK(count_bad / count_reps < doctest::Approx(chance_of_failure_0_01));
} // TEST_CASE_TEMPLATE(...)

TEST_SUITE("Benchmarks")
{
    TEST_CASE_TEMPLATE("benchmarking (randomized) normal_sampler_512 vs STL", tested_t, ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_ALL_TYPES)
    {
        using engine_type = typename tested_t::engine_type;
        using value_type = typename tested_t::value_type;
        using probability_type = typename tested_t::probability_type;
        using expectation_type = typename tested_t::expectation_type;
        using sampler_type = ropufu::aftermath::random::normal_sampler_512<engine_type, value_type, probability_type, expectation_type>;
        
        if (!ropufu::aftermath::tests::g_do_benchmarks) return;

        constexpr std::size_t sample_size = 1'048'576;

        double aftermath_ms = std::numeric_limits<double>::infinity();
        double builtin_ms = std::numeric_limits<double>::infinity();
        ropufu::aftermath::tests::timing_vs_builtin<engine_type, sampler_type>(sample_size, aftermath_ms, builtin_ms);

        BENCH_COMPARE_TIMING(aftermath_ms, builtin_ms);
    } // TEST_CASE_TEMPLATE(...)
} // TEST_SUITE(..)

#endif // ROPUFU_AFTERMATH_TESTS_RANDOM_NORMAL_SAMPLER_512_HPP_INCLUDED
